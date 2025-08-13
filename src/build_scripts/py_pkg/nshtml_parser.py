from typing import Dict, List, Optional, Self, Set, Tuple
from dataclasses import dataclass


def html_encode(s: str) -> str:
    s = s.replace("&", "&amp;")
    s = s.replace('"', "&quot;")
    # TODO: all characters
    return s


type HtmlElement = HtmlTag | HtmlText
type HtmlTextPart = str | HtmlNSPlaceholder
type Params = Dict[str, Html]


@dataclass
class Html:
    contents: List[HtmlElement]

    def compress(self):
        new_content = []
        txt_accum = None
        for e in self.contents:
            if isinstance(e, HtmlText):
                if txt_accum is None:
                    txt_accum = e
                else:
                    txt_accum.append(e)
            else:
                if txt_accum is not None:
                    new_content.append(txt_accum)
                    txt_accum = None
                new_content.append(e)

        if txt_accum is not None:
            new_content.append(txt_accum)
        self.contents = new_content

    def apply_placeholders(self, params: Params, deps: List[str]):
        elts = []
        for e in self.contents:
            if isinstance(e, HtmlText):
                elts += e.apply_placeholders(params, deps)
            else:
                e.apply_placeholders(params, deps)
                elts.append(e)
        self.contents = elts

    @classmethod
    def parse(cls, s: str, closing: Optional[str] = None) -> Tuple[str, Self]:
        elts: List[HtmlElement] = []
        while len(s) > 0:
            if s.startswith("<"):
                s, tag = HtmlTag.parse(s)
                if closing is not None:
                    if (tag.kind & HTML_TAG_CLOSE) != 0 and tag.name == closing:
                        break
                elts.append(tag)
            else:
                s, text = HtmlText.parse(s)
                elts.append(text)

        return s, cls(elts)

    def __str__(self) -> str:
        return "".join([str(elt) for elt in self.contents])


@dataclass
class HtmlStrlit:
    contents: List[HtmlTextPart]

    def to_html(self) -> Html:
        s = "".join([str(c) for c in self.contents])
        _, html = Html.parse(s)
        return html

    def apply_placeholders(self, params: Params, deps: List[str]):
        cnts = []
        out = ""
        for c in self.contents:
            if isinstance(c, str):
                out += c
                continue
            assert isinstance(c, HtmlNSPlaceholder)
            out += str(c.apply(params, deps))

        if len(out) > 0:
            cnts.append(out)
        self.contents = cnts
        _, new_one = HtmlStrlit.parse(str(self))
        self.contents = new_one.contents
        return self

    @classmethod
    def parse(cls, s: str) -> Tuple[str, Self]:
        assert s[0] == '"'
        s = s[1:]
        out = ""
        elts = []
        while len(s) > 0:
            if s[0] == '"':
                break
            if s.startswith("{{"):
                if len(out) > 0:
                    elts.append(out)
                    out = ""
                s, ph = HtmlNSPlaceholder.parse(s)
                elts.append(ph)
                continue
            if s[0] == "\\":
                assert len(s) > 1
                s = s[1:]
            out += s[0]
            s = s[1:]
        if len(out) > 0:
            elts.append(out)
        return s[1:], cls(elts)

    def __str__(self) -> str:
        return '"' + "".join([str(c).replace('"', '\\"') for c in self.contents]) + '"'


HTML_TAG_OPEN = 0
HTML_TAG_CLOSE = 1
HTML_TAG_SELFCLOSE = 2
HTML_TAG_COMPONENT = 4
HTML_TAG_COMMENT = 999
HTML_TAG_DOCTYPE = 1000


@dataclass
class HtmlTag:
    name: str
    attrs: Dict[str, Optional[HtmlStrlit]]
    placeholders: List["HtmlNSPlaceholder"]
    kind: int
    child: Optional[Html]

    def apply_placeholders(self, params: Params, deps: List[str]):
        if self.kind == HTML_TAG_COMMENT or self.kind == HTML_TAG_DOCTYPE:
            return
        new_attrs = {}
        new_phs = []
        for k, v in self.attrs.items():
            if v is not None:
                v = v.apply_placeholders(params, deps)
            new_attrs[k] = v
        for ph in self.placeholders:
            s = str(ph.apply(params, deps)).strip()
            _, o_attrs, o_phs = HtmlTag.parse_attr_list(s)
            new_phs += o_phs
            new_attrs.update(o_attrs)
        self.attrs = new_attrs
        self.placeholders = new_phs
        if self.child is not None:
            self.child.apply_placeholders(params, deps)

    @classmethod
    def parse_ident(cls, s: str) -> Tuple[str, str]:
        ident_chars = (
            list(range(ord("a"), ord("z") + 1))
            + list(range(ord("A"), ord("Z") + 1))
            + list(range(ord("0"), ord("9") + 1))
            + [ord("_")]
        )
        ident = ""
        while ord(s[0]) in ident_chars:
            ident += s[0]
            s = s[1:]
        return s, ident

    @classmethod
    def parse_tag_name(cls, s: str) -> Tuple[str, str, bool]:
        if not s.startswith("{"):
            s, i = cls.parse_ident(s)
            return s, i, False
        s = s[1:]
        name = ""
        while not s.startswith("}"):
            name += s[0]
            s = s[1:]
        return s[1:], name, True

    @classmethod
    def parse_attr(cls, s: str) -> Tuple[str, str, Optional[HtmlStrlit]]:
        s, k = cls.parse_ident(s)
        s = s.strip()
        if not s.startswith("="):
            return s, k, None
        s = s[1:].strip()
        s, v = HtmlStrlit.parse(s)
        return s, k, v

    @classmethod
    def parse_attr_list(
        cls, s: str
    ) -> Tuple[str, Dict[str, Optional[HtmlStrlit]], List["HtmlNSPlaceholder"]]:
        phs = []
        attrs = {}
        while len(s) > 0:
            if s[0] == ">" or s[0] == "/":
                break
            if s.startswith("{{"):
                s, ph = HtmlNSPlaceholder.parse(s)
                phs.append(ph)
                s = s.strip()
                continue
            s, k, v = cls.parse_attr(s)

            attrs[k] = v
            s = s.strip()
        return s, attrs, phs

    @classmethod
    def parse(cls, s: str) -> Tuple[str, Self]:
        assert s[0] == "<"
        kind = 0
        s = s[1:].strip()
        if s.startswith("!--"):
            s = s[3:]
            kind = HTML_TAG_COMMENT
            while not s.startswith("-->"):
                s = s[1:]
            return s[3:], cls("", {}, [], kind, None)
        if s.startswith("!DOCTYPE"):
            kind = HTML_TAG_DOCTYPE
            s = s[8:].strip()
            name = ""
            while s[0] != ">":
                name += s[0]
                s = s[1:]
            return s[1:], cls(name.strip(), {}, [], kind, None)
        if s[0] == "/":
            kind |= HTML_TAG_CLOSE
            s = s[1:].strip()
        s, name, is_component = cls.parse_tag_name(s)
        if is_component:
            kind |= HTML_TAG_COMPONENT
        s, attrs, phs = cls.parse_attr_list(s.strip())
        if s.startswith("/"):
            kind |= HTML_TAG_SELFCLOSE
            s = s[1:].strip()
        assert s[0] == ">"
        s = s[1:].strip()
        body = None
        if (kind & (HTML_TAG_CLOSE | HTML_TAG_SELFCLOSE)) == 0:
            s, body = Html.parse(s, name)
        return s, cls(name, attrs, phs, kind, body)

    def str_no_body(self) -> str:
        if self.kind == HTML_TAG_COMMENT:
            return ""
        if self.kind == HTML_TAG_DOCTYPE:
            return f"<!DOCTYPE {self.name}>"
        start = "<"
        if (self.kind & HTML_TAG_CLOSE) != 0:
            start = "</"
        end = ">"
        if self.kind & HTML_TAG_SELFCLOSE:
            end = "/>"
        tag_name = self.name
        if self.kind & HTML_TAG_COMPONENT:
            tag_name = "{" + tag_name + "}"
        tag = start + tag_name
        for k, v in self.attrs.items():
            if v is None:
                tag += f" {k}"
            else:
                tag += f" {k}={v}"
        for ph in self.placeholders:
            tag += f" {ph}"
        tag = tag + end
        return tag

    def closing_str(self) -> str:
        if self.kind == HTML_TAG_COMMENT or self.kind == HTML_TAG_DOCTYPE:
            return ""
        tag_name = self.name
        if self.kind & HTML_TAG_COMPONENT:
            tag_name = "{" + tag_name + "}"
        return f"</{tag_name}>"

    def __str__(self) -> str:
        if self.kind == HTML_TAG_COMMENT:
            return ""
        tag = self.str_no_body()
        if self.child is not None:
            tag += str(self.child) + self.closing_str()
        return tag


@dataclass
class HtmlText:
    contents: List[HtmlTextPart]

    def append(self, other: "HtmlText"):
        if len(other.contents) == 0:
            return
        if len(self.contents) == 0:
            self.contents = other.contents
            return
        if isinstance(other.contents[0], HtmlNSPlaceholder) or isinstance(
            self.contents[-1], HtmlNSPlaceholder
        ):
            self.contents += other.contents
            return
        lst = self.contents[-1]
        self.contents = self.contents[:-1]
        fst = other.contents[0]
        other.contents = other.contents[1:]
        self.contents.append(lst + fst)
        self.contents += other.contents

    def apply_placeholders(self, params: Params, deps: List[str]) -> List[HtmlElement]:
        raw_elts: List[HtmlElement] = []
        for c in self.contents:
            if isinstance(c, str):
                raw_elts.append(HtmlText([c]))
                continue
            assert isinstance(c, HtmlNSPlaceholder)
            h = c.apply(params, deps)
            raw_elts += h.contents
        html = Html(raw_elts)
        html.compress()
        return html.contents

    @classmethod
    def parse(cls, s: str) -> Tuple[str, Self]:
        out = ""
        elts = []
        s = s.strip()
        while len(s) > 0:
            if s[0] == "<":
                break
            if s.startswith("{{"):
                if len(out) > 0:
                    elts.append(out)
                    out = ""
                s, ph = HtmlNSPlaceholder.parse(s)
                elts.append(ph)
                continue
            if s[0].isspace():
                out += " "
                s = s.strip()
                continue
            out += s[0]
            s = s[1:]
        if len(out) > 0:
            elts.append(out.rstrip())
        return s, cls(elts)

    def __str__(self):
        return "".join([str(c) for c in self.contents])


@dataclass
class HtmlNSPlaceholder:
    name: str
    is_optional: bool
    replace_if_true: Optional[Html]
    replace_if_false: Optional[Html]

    def apply(self, params: Params, deps: List[str]) -> Html:
        if self.replace_if_true is not None:
            self.replace_if_true.apply_placeholders(params, deps)
        if self.replace_if_false is not None:
            self.replace_if_false.apply_placeholders(params, deps)
        if self.name in deps or (
            not self.is_optional and self.name not in params.keys()
        ):
            return Html([HtmlText([self])])
        if self.name in params.keys():
            if self.replace_if_true is not None:
                return self.replace_if_true
            return params[self.name]
        if self.replace_if_false is not None:
            return self.replace_if_false
        return Html([])

    @classmethod
    def balanced_text(cls, text: str) -> Tuple[str, str]:
        out = ""
        assert text.startswith("{{")
        out += text[:2]
        text = text[2:]
        depth = 1
        while len(text) > 0:
            if text.startswith("{{"):
                out += text[:2]
                text = text[2:]
                depth += 1
            elif text.startswith("}}"):
                out += text[:2]
                text = text[2:]
                depth -= 1
                if depth == 0:
                    break
            else:
                out += text[0]
                text = text[1:]
        return text, out

    @classmethod
    def parse(cls, s: str) -> Tuple[str, Self]:
        assert s.startswith("{{")
        s = s[2:].strip()
        name = ""
        while not s[0].isspace() and not s.startswith("}}") and s[0] != "?":
            name += s[0]
            s = s[1:]
        s = s.strip()
        if s[0] != "?":
            assert s.startswith("}}")
            return s[2:], cls(name, False, None, None)
        s = s[1:]
        s = s.strip()
        if s.startswith("}}"):
            return s[2:], cls(name, True, None, None)
        true_html = None
        if s.startswith("{{"):
            s, inner = cls.balanced_text(s)
            _, true_html = Html.parse(inner[2:-2])
            s = s.strip()
        if s.startswith("}}"):
            return s[2:], cls(name, True, true_html, None)
        assert s[0] == "?"
        s = s[1:].strip()
        assert s.startswith("{{")
        s, inner = cls.balanced_text(s)
        _, false_html = Html.parse(inner[2:-2])
        s = s.strip()
        assert s.startswith("}}")
        return s[2:], cls(name, True, true_html, false_html)

    def __str__(self):
        out = "{{" + self.name
        if self.is_optional:
            out += "?"
        if self.replace_if_true is not None:
            out += "{{" + str(self.replace_if_true) + "}}"
        if self.replace_if_false is not None:
            out += "?{{" + str(self.replace_if_false) + "}}"
        return out + "}}"


ROOT = "."


def read_html_file(filename: str) -> Html:
    with open(ROOT + "/" + filename, "r") as f:
        _, html = Html.parse(f.read())
        return html


def get_component_dependencies(html: Html) -> Set[str]:
    def handle_ph(t: HtmlNSPlaceholder) -> Set[str]:
        out = set()
        out.add(t.name)
        if t.replace_if_true is not None:
            out.update(get_component_dependencies(t.replace_if_true))
        if t.replace_if_false is not None:
            out.update(get_component_dependencies(t.replace_if_false))
        return out

    def handle_text_or_strlit(t: HtmlText | HtmlStrlit):
        out = set()
        for e in t.contents:
            if isinstance(e, HtmlNSPlaceholder):
                out.update(handle_ph(e))
        return out

    out = set()
    for e in html.contents:
        if isinstance(e, HtmlText):
            out.update(handle_text_or_strlit(e))
        elif isinstance(e, HtmlTag):
            if e.child is not None:
                out.update(get_component_dependencies(e.child))
            for _, v in e.attrs.items():
                if v is None:
                    continue
                out.update(handle_text_or_strlit(v))
            for ph in e.placeholders:
                out.update(handle_ph(ph))
    return out


def get_component_info(tag: HtmlTag) -> Tuple[str, Params, List[str]]:
    assert (tag.kind & HTML_TAG_COMPONENT) != 0
    filename = tag.name.replace("::", "/") + ".html"
    params = {}
    for k, v in tag.attrs.items():
        if v is None:
            params[k] = Html([])
        else:
            params[k] = v.to_html()
    if tag.child is not None:
        params["children"] = tag.child
    deps = []
    for ph in tag.placeholders:
        if ph.name == str(ph.replace_if_true):
            deps.append(ph.name)
    return filename, params, deps


def apply_components(html: Html, params: Params, deps: List[str]) -> Html:
    elts = []
    for e in html.contents:
        if isinstance(e, HtmlText):
            elts.append(e)
            continue
        assert isinstance(e, HtmlTag), f"GOT {e.__class__}"
        if e.kind == HTML_TAG_COMMENT:
            continue
        if e.child is not None:
            e.child = apply_components(e.child, params, deps)
        if (e.kind & HTML_TAG_COMPONENT) == 0:
            elts.append(e)
            continue
        comp_filename, comp_params, comp_deps = get_component_info(e)
        comp_html = read_html_file(comp_filename)
        comp_html.apply_placeholders(comp_params, comp_deps)
        comp = apply_components(comp_html, comp_params, comp_deps)
        elts += comp.contents

    return Html(elts)


def get_main_html(filename: str) -> Html:
    html = read_html_file(filename)
    deps = list(get_component_dependencies(html))
    html.apply_placeholders(dict(), deps)
    a = apply_components(html, dict(), deps)
    return a


def pprint_html(html: Html, indent=0):
    for e in html.contents:
        if isinstance(e, HtmlText):
            s = str(e)
            if len(s) != 0:
                print(" " * indent, s, sep="")
            continue
        print(" " * indent, e.str_no_body(), sep="")
        if e.child is not None:
            pprint_html(e.child, indent + 2)
            print(" " * indent, e.closing_str(), sep="")


def compile_html(from_dir, to_dir, filename):
    global ROOT
    ROOT = from_dir
    html = get_main_html(filename)
    with open(to_dir + "/" + filename, "w") as f:
        f.write(str(html))


def main():
    import sys

    ROOT = sys.argv[1]
    html = get_main_html(sys.argv[2])
    pprint_html(html)


if __name__ == "__main__":
    main()
