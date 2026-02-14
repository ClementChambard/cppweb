from dataclasses import dataclass
from enum import Enum, auto

from .token import DoctypeData, PlaceholderData, TagData, Token, Tok
from .named_chars import check_after_amp
from util.charmanip import (
    is_surrogate,
    is_nonchar,
    is_ascii_whitespace,
    normalize_newlines,
    is_control,
    CC_MAP,
)

VOID_ELEMENT_KINDS = [
    "area",
    "base",
    "br",
    "col",
    "embed",
    "hr",
    "img",
    "input",
    "link",
    "meta",
    "source",
    "track",
    "wbr",
]
TEMPLATE_ELEM_ENT_KINDS = ["template"]
RAW_TEXT_ELEMENT_KINDS = ["script", "style"]
ESCAPABLE_RAW_TEXT_ELEMENT_KINDS = ["textarea", "title"]


class TMode(Enum):
    NORMAL = auto()
    RCDATA = auto()
    RAWTEXT = auto()
    SCRIPTDATA = auto()
    PLAINTEXT = auto()


@dataclass
class Tokenizer:
    stream: str
    mode: TMode

    def __init__(self, stream: str):
        self.stream = normalize_newlines(stream)
        self.mode = TMode.NORMAL

    def error(self, name: str):
        print("ERROR:", name)

    def tok(self) -> Token:
        if self.mode == TMode.RCDATA:
            return self.tok_rcdata()
        elif self.mode == TMode.RAWTEXT:
            return self.tok_rawtext()
        elif self.mode == TMode.SCRIPTDATA:
            return self.tok_script_data()
        elif self.mode == TMode.PLAINTEXT:
            return self.tok_plaintext()
        if len(self.stream) == 0:
            return Token(Tok.EOF, None)
        c = self.stream[0]
        if c == "&":
            if self.stream[1].isalpha():
                return Token(Tok.CHARACTER, self.read_alpha_char_ref())
            elif self.stream[1:3].lower() == "#x":
                return Token(Tok.CHARACTER, self.read_num_char_ref(16, self.stream[3:]))
            elif self.stream[1] == "#":
                return Token(Tok.CHARACTER, self.read_num_char_ref(10, self.stream[2:]))
        elif c == "<":
            self.stream = self.stream[1:]
            if len(self.stream) == 0:
                self.error("eof-before-tag-name")
                return Token(Tok.CHARACTER, c)
            elif self.stream[0] == "!":
                if self.stream[1:].startswith("--"):
                    self.stream = self.stream[3:]
                    return self.read_comment()
                elif self.stream[1:].startswith("[CDATA["):
                    assert False, "TODO: <![CDATA["
                elif self.stream[1:8].lower() == "doctype":
                    self.stream = self.stream[8:]
                    return self.read_doctype()
                else:
                    self.error("incorrectly-opened-comment")
                    return self.read_bogus_comment()
            elif self.stream[0] == "/":
                self.stream = self.stream[1:]
                if len(self.stream) == 0:
                    self.error("eof-before-tag-name")
                    self.stream = "/"
                    return Token(Tok.CHARACTER, "<")
                elif self.stream[0] == ">":
                    self.stream = self.stream[1:]
                    self.error("missing-end-tag-name")
                    return self.tok()
                elif self.stream[0] == "{" or self.stream[0].isalpha():
                    is_component = self.stream[0] == "{"
                    tag = self.read_tag(is_component)
                    if tag is None:
                        return Token(Tok.EOF, None)
                    if len(tag.attributes) > 0:
                        self.error("end-tag-with-attributes")
                    if tag.self_closing:
                        self.error("end-tag-with-trailing-solidus")
                    return Token([Tok.END_TAG, Tok.END_COMPONENT][is_component], tag)
                else:
                    self.error("invalid-first-character-of-tag-name")
                    return self.read_bogus_comment()
            elif self.stream[0] == "{" or self.stream[0].isalpha():
                is_component = self.stream[0] == "{"
                tag = self.read_tag(is_component)
                if tag is None:
                    return Token(Tok.EOF, None)
                if tag.name not in VOID_ELEMENT_KINDS and tag.self_closing:
                    self.error("non-void-html-element-start-tag-with-trailing-solidus")
                return Token([Tok.START_TAG, Tok.START_COMPONENT][is_component], tag)
            elif self.stream[0] == "?":
                self.stream = self.stream[1:]
                return self.read_bogus_comment()
            else:
                self.error("invalid-first-character-of-tag-name")
                return Token(Tok.CHARACTER, c)
        elif c == "\0":
            self.error("unexpected-null-character")
        elif self.stream.startswith("{{"):
            ph = self.read_placeholder()
            return Token(Tok.PLACEHOLDER, ph)
        self.stream = self.stream[1:]
        return Token(Tok.CHARACTER, c)

    def detect_end_tag(self):
        if len(self.stream) == 0 or self.stream[0] != "<":
            return False
        if len(self.stream) == 1 or self.stream[1] != "/":
            return False
        if len(self.stream) == 2 or not self.stream[2].isalpha():
            return False
        is_ok = True
        i = 2
        while True:
            if len(self.stream) <= i:
                is_ok = False
                break
            if self.stream[i].isalpha():
                i += 1
                continue
            if self.stream[i] in "\t\n\f />":
                break
            is_ok = False
            break
        return is_ok

    def tok_rcdata(self) -> Token:
        if len(self.stream) == 0:
            return Token(Tok.EOF, None)
        if self.stream[0] == "&":
            if self.stream[1].isalpha():
                return Token(Tok.CHARACTER, self.read_alpha_char_ref())
            elif self.stream[1:3].lower() == "#x":
                return Token(Tok.CHARACTER, self.read_num_char_ref(16, self.stream[3:]))
            elif self.stream[1] == "#":
                return Token(Tok.CHARACTER, self.read_num_char_ref(10, self.stream[2:]))
        elif self.detect_end_tag():
            self.mode = TMode.NORMAL
            return self.tok()
        c, self.stream = self.stream[0], self.stream[1:]
        if c == "\0":
            self.error("unexpected-null-character")
            c = "\ufffd"
        return Token(Tok.CHARACTER, c)

    def tok_rawtext(self) -> Token:
        if self.detect_end_tag():
            self.mode = TMode.NORMAL
            return self.tok()
        return self.tok_plaintext()

    def tok_script_data(self) -> Token:
        if self.stream.startswith("<!"):
            self.stream = self.stream[1:]
            return Token(Tok.CHARACTER, "<")
            # TODO: escape
        elif self.detect_end_tag():
            self.mode = TMode.NORMAL
            return self.tok()
        return self.tok_plaintext()

    def tok_plaintext(self) -> Token:
        if len(self.stream) == 0:
            return Token(Tok.EOF, None)
        c, self.stream = self.stream[0], self.stream[1:]
        if c == "\0":
            self.error("unexpected-null-character")
            c = "\ufffd"
        return Token(Tok.CHARACTER, c)

    def read_comment(self) -> Token:
        s = ""
        if self.stream.startswith("->"):
            self.stream = self.stream[2:]
            self.error("abrupt-closing-of-empty-comment")
            return Token(Tok.COMMENT, s)
        elif self.stream.startswith(">"):
            self.stream = self.stream[1:]
            self.error("abrupt-closing-of-empty-comment")
            return Token(Tok.COMMENT, s)
        while True:
            if len(self.stream) == 0:
                self.error("eof-in-comment")
                break
            if self.stream.startswith("--!>"):
                self.error("incorrectly-closed-comment")
                self.stream = self.stream[4:]
                break
            if self.stream.startswith("<!-->"):  # WHY no error here ???
                self.stream = self.stream[5:]
                break
            if self.stream.startswith("-->"):
                self.stream = self.stream[3:]
                break
            if self.stream.startswith("<!--"):
                self.error("nested-comment")
                self.stream = self.stream[2:]  # dont copy the chars ?????
                continue
            s += self.stream[0]
            self.stream = self.stream[1:]
        return Token(Tok.COMMENT, s)

    def read_doctype(self) -> Token:
        if len(self.stream) == 0:
            self.error("eof-in-doctype")
            return Token(Tok.DOCTYPE, DoctypeData(None, None, None, True))
        if self.stream[0] not in "\t\n\f >":
            self.error("missing-whitespace-before-doctype-name")
        self.skip_space()
        if self.stream.startswith(">"):
            self.stream = self.stream[1:]
            self.error("missing-doctype-name")
            return Token(Tok.DOCTYPE, DoctypeData(None, None, None, True))
        doctype = DoctypeData("", None, None, False)
        assert doctype.name is not None
        while True:
            if len(self.stream) == 0:
                self.error("eof-in-doctype")
                doctype.force_quirks = True
                return Token(Tok.DOCTYPE, doctype)
            elif self.stream[0] in "\t\n\f ":
                self.stream = self.stream[1:]
                break
            elif self.stream[0] == ">":
                self.stream = self.stream[1:]
                return Token(Tok.DOCTYPE, doctype)
            elif self.stream[0] == "\0":
                self.stream = self.stream[1:]
                self.error("unexpected-null-character")
                doctype.name += "\ufffd"
            else:
                doctype.name += self.stream[0].lower()
                self.stream = self.stream[1:]
        self.skip_space()
        if len(self.stream) == 0:
            self.error("eof-in-doctype")
            doctype.force_quirks = True
            return Token(Tok.DOCTYPE, doctype)
        if self.stream[0] == ">":
            self.stream = self.stream[1:]
            return Token(Tok.DOCTYPE, doctype)
        if len(self.stream) >= 6 and self.stream[:6].lower() == "public":
            self.stream = self.stream[6:]
            if not (len(self.stream) == 0 or self.stream[0] in "\t\n\f "):
                if self.stream[0] == '"':
                    self.error("missing-whitespace-after-doctype-public-keyword")
                elif self.stream[0] == "'":
                    self.error("missing-whitespace-after-doctype-public-keyword")
                elif self.stream[0] == ">":
                    self.stream = self.stream[1:]
                    self.error("missing-doctype-public-identifier")
                    doctype.force_quirks = True
                    return Token(Tok.DOCTYPE, doctype)
                else:
                    self.error("missing-quote-before-doctype-public-identifier")
                    return self.read_bogus_doctype(doctype)
            public_ident = self.read_doctype_ident(
                "missing-doctype-public-identifier",
                "missing-quote-before-doctype-public-identifier",
                "abrupt-doctype-public-identifier",
            )
            if public_ident is None:
                return self.read_bogus_doctype(doctype)
            doctype.public_ident = public_ident
            if not (len(self.stream) == 0 or self.stream[0] in "\t\n\f "):
                if self.stream[0] == '"':
                    self.error(
                        "missing-whitespace-between-doctype-public-and-system-identifiers"
                    )
                elif self.stream[0] == "'":
                    self.error(
                        "missing-whitespace-between-doctype-public-and-system-identifiers"
                    )
                elif self.stream[0] == ">":
                    self.stream = self.stream[1:]
                    return Token(Tok.DOCTYPE, doctype)
                else:
                    self.error("missing-quote-before-doctype-system-identifier")
                    return self.read_bogus_doctype(doctype)
            self.skip_space()
            if len(self.stream) == 0:
                self.error("eof-in-doctype")
                doctype.force_quirks = True
                return Token(Tok.DOCTYPE, doctype)
            elif self.stream[0] == ">":
                self.stream = self.stream[1:]
                return Token(Tok.DOCTYPE, doctype)
            system_ident = self.read_doctype_ident(
                "missing-doctype-system-identifier",
                "missing-quote-before-doctype-system-identifier",
                "abrupt-doctype-system-identifier",
            )
            if system_ident is None:
                return self.read_bogus_doctype(doctype)
            doctype.system_ident = system_ident
            self.skip_space()
            if len(self.stream) == 0:
                self.error("eof-in-doctype")
                doctype.force_quirks = True
                return Token(Tok.DOCTYPE, doctype)
            elif self.stream[0] == ">":
                self.stream = self.stream[1:]
                return Token(Tok.DOCTYPE, doctype)
            else:
                self.error("unexpected-character-after-doctype-system-identifier")
                return self.read_bogus_doctype(doctype)
        elif len(self.stream) >= 6 and self.stream[:6].lower() == "system":
            self.stream = self.stream[6:]
            if not (len(self.stream) == 0 or self.stream[0] in "\t\n\f "):
                if self.stream[0] == '"':
                    self.error("missing-whitespace-after-doctype-system-keyword")
                elif self.stream[0] == "'":
                    self.error("missing-whitespace-after-doctype-system-keyword")
                elif self.stream[0] == ">":
                    self.stream = self.stream[1:]
                    self.error("missing-doctype-system-identifier")
                    doctype.force_quirks = True
                    return Token(Tok.DOCTYPE, doctype)
                else:
                    self.error("missing-quote-before-doctype-system-identifier")
                    return self.read_bogus_doctype(doctype)
            system_ident = self.read_doctype_ident(
                "missing-doctype-system-identifier",
                "missing-quote-before-doctype-system-identifier",
                "abrupt-doctype-system-identifier",
            )
            if system_ident is None:
                return self.read_bogus_doctype(doctype)
            doctype.system_ident = system_ident
            self.skip_space()
            if len(self.stream) == 0:
                self.error("eof-in-doctype")
                doctype.force_quirks = True
                return Token(Tok.DOCTYPE, doctype)
            elif self.stream[0] == ">":
                self.stream = self.stream[1:]
                return Token(Tok.DOCTYPE, doctype)
            else:
                self.error("unexpected-character-after-doctype-system-identifier")
                return self.read_bogus_doctype(doctype)
        else:
            self.error("invalid-character-sequence-after-doctype-name")
            return self.read_bogus_doctype(doctype)

    def read_bogus_doctype(self, doctype: DoctypeData) -> Token:
        doctype.force_quirks = True
        while True:
            if len(self.stream) == 0:
                return Token(Tok.DOCTYPE, doctype)
            elif self.stream[0] == ">":
                self.stream = self.stream[1:]
                return Token(Tok.DOCTYPE, doctype)
            elif self.stream[0] == "\0":
                self.error("unexpected-null-character")
            self.stream = self.stream[1:]

    def read_doctype_ident(
        self, missing_error, missing_quote_error, abrupt_error
    ) -> str | None:
        self.skip_space()
        if len(self.stream) == 0:
            self.error("eof-in-doctype")
            return None
        if self.stream[0] == ">":
            self.error(missing_error)
            return None
        quot = ""
        if self.stream[0] == "'":
            quot = "'"
        elif self.stream[0] == '"':
            quot = '"'
        else:
            self.error(missing_quote_error)
            return None
        out = ""
        while True:
            if len(self.stream) == 0:
                self.error("eof-in-doctype")
                return None
            elif self.stream[0] == quot:
                self.stream = self.stream[1:]
                break
            elif self.stream[0] == "\0":
                self.stream = self.stream[1:]
                self.error("unexpected-null-character")
                out += "\ufffd"
            elif self.stream[0] == ">":
                self.stream = self.stream[1:]
                self.error(abrupt_error)
                return None
            else:
                out += self.stream[0]
                self.stream = self.stream[1:]
        return out

    def skip_space(self):
        while len(self.stream) > 0 and self.stream[0] in "\n\f\t ":
            self.stream = self.stream[1:]

    def read_tag(self, is_component: bool) -> TagData | None:
        if is_component:
            assert self.stream[0] == "{"
            self.stream = self.stream[1:]
        data = TagData(self.stream[0], False, {}, [])
        while True:
            self.stream = self.stream[1:]
            if len(self.stream) == 0:
                self.error("eof-in-tag")
                return None
            if is_component:
                if self.stream[0] == "}":
                    self.stream = self.stream[1:]
                    break
            else:
                if self.stream[0] in "\t\n\f />":
                    break
            if self.stream[0] == "\0":
                self.error("unexpected-null-character")
                data.name += "\ufffd"
            else:
                data.name += self.stream[0].lower()
        while True:
            self.skip_space()
            if len(self.stream) == 0:
                self.error("eof-in-tag")
                return None
            if self.stream[0] == "/":
                self.stream = self.stream[1:]
                if len(self.stream) == 0:
                    self.error("eof-in-tag")
                    return None
                elif self.stream[0] == ">":
                    self.stream = self.stream[1:]
                    data.self_closing = True
                    return data
                else:
                    self.error("unexpected-solidus-in-tag")
                    continue
            if self.stream[0] == ">":
                self.stream = self.stream[1:]
                return data
            if self.stream.startswith("{{"):
                data.placeholders.append(self.read_placeholder())
                continue
            cur_attribute_name = ""
            cur_attribute_value = ""
            if self.stream[0] == "=":
                self.error("unexpected-equals-sign-before-attribute-name")
                cur_attribute_name = "="
                self.stream = self.stream[1:]
            while len(self.stream) > 0 and self.stream[0] not in "/>\t\n\f =":
                if self.stream[0] == "\0":
                    self.error("unexpected-null-character")
                    cur_attribute_name += "\ufffd"
                else:
                    if self.stream[0] in "\"'<":
                        self.error("unexpected-character-in-attribute-name")
                    cur_attribute_name += self.stream[0]
                self.stream = self.stream[1:]
            assert cur_attribute_name != ""
            self.skip_space()
            data.attributes[cur_attribute_name] = []
            if len(self.stream) == 0 or self.stream[0] != "=":
                continue
            self.stream = self.stream[1:]
            self.skip_space()
            quot = "\n\t\f >"
            if len(self.stream) == 0:
                pass
            elif self.stream[0] == '"':
                quot = '"'
                self.stream = self.stream[1:]
            elif self.stream[0] == "'":
                quot = "'"
                self.stream = self.stream[1:]
            elif self.stream[0] == ">":
                self.error("missing-attribute-value")
                self.stream = self.stream[1:]
                return data
            while True:
                if len(self.stream) == 0:
                    self.error("eof-in-tag")
                    return None
                c = self.stream[0]
                if self.stream.startswith("{{"):
                    if cur_attribute_value != "":
                        data.attributes[cur_attribute_name].append(cur_attribute_value)
                        cur_attribute_value = ""
                        data.attributes[cur_attribute_name].append(self.read_placeholder())
                if c in quot:
                    self.stream = self.stream[1:]
                    break
                if c == "&":
                    if self.stream[1].isalpha():
                        c = self.read_alpha_char_ref(True)
                    elif self.stream[1:3].lower() == "#x":
                        c = self.read_num_char_ref(16, self.stream[3:])
                    elif self.stream[1] == "#":
                        c = self.read_num_char_ref(10, self.stream[2:])
                    else:
                        self.stream = self.stream[1:]
                elif c == "\0":
                    self.stream = self.stream[1:]
                    self.error("unexpected-null-character")
                    c = "\ufffd"
                elif quot == "\n\t\f >" and c in "\"'<=`":
                    self.error("unexpected-character-in-unquoted-attribute-value")
                    self.stream = self.stream[1:]
                else:
                    self.stream = self.stream[1:]
                cur_attribute_value += c
            if cur_attribute_value != "" or data.attributes[cur_attribute_value] == []:
                data.attributes[cur_attribute_name].append(cur_attribute_value)
            if len(self.stream) == 0 or quot == "\n\t\f >" or self.stream[0] in "/>":
                continue
            if self.stream[0] not in "\n\t\f ":
                self.error("missing-whitespace-between-attributes")

    def read_placeholder(self) -> PlaceholderData:
        assert self.stream.startswith("{{")
        self.stream = self.stream[2:]
        self.skip_space()
        name = ""
        while True:
            if len(self.stream) == 0:
                self.error("eof-in-placeholder")
            if self.stream.startswith("}}") or self.stream[0] in "\t\n\f ?":
                break
            name += self.stream[0]
            self.stream = self.stream[1:]
        if name == "":
            self.error("empty-placeholder-name")
        self.skip_space()
        if self.stream.startswith("}}"):
            self.stream = self.stream[2:]
            return PlaceholderData(name, False, None, None)
        if self.stream[0] != "?":
            self.error("invalid-placeholder")
            return PlaceholderData(name, False, None, None)
        self.stream = self.stream[1:]
        self.skip_space()
        if self.stream.startswith("}}"):
            self.stream = self.stream[2:]
            return PlaceholderData(name, True, None, None)
        replace_if_true = []
        replace_if_false = []
        self.skip_space()
        if self.stream.startswith("{{"):
            self.stream = self.stream[2:]
            while not self.stream.startswith("}}"):
                if len(self.stream) == 0:
                    self.error("eof-in-placeholder")
                replace_if_true.append(self.tok())
            assert self.stream.startswith("}}")
            self.stream = self.stream[2:]
            self.skip_space()
            if self.stream.startswith("}}"):
                self.stream = self.stream[2:]
                return PlaceholderData(name, True, replace_if_true, replace_if_false)
        if self.stream[0] != "?":
            self.error("invalid-placeholder")
        self.stream = self.stream[1:]
        self.skip_space()
        if not self.stream.startswith("{{"):
            self.error("invalid-placeholder")
        self.stream = self.stream[2:]
        while not self.stream.startswith("}}"):
            if len(self.stream) == 0:
                self.error("eof-in-placeholder")
            replace_if_false.append(self.tok())
        assert self.stream.startswith("}}")
        self.stream = self.stream[2:]
        self.skip_space()
        if not self.stream.startswith("}}"):
            self.error("unclosed-placeholder")
        self.stream = self.stream[2:]
        return PlaceholderData(name, True, replace_if_true, replace_if_false)

    def read_bogus_comment(self) -> Token:
        s = ""
        while True:
            if len(self.stream) == 0:
                return Token(Tok.COMMENT, s)
            elif self.stream[0] == ">":
                self.stream = self.stream[1:]
                return Token(Tok.COMMENT, s)
            elif self.stream[0] == "\0":
                self.error("unexpected-null-character")
                s += "\ufffd"
                self.stream = self.stream[1:]
            else:
                s += self.stream[0]
                self.stream = self.stream[1:]

    def read_alpha_char_ref(self, part_of_attribute=False) -> str:
        out = check_after_amp(self.stream[1:])
        if out is None:
            self.stream = self.stream[1:]
            return "&"
        new_stream, chars, has_semi = out
        if (
            part_of_attribute
            and not has_semi
            and len(new_stream) > 0
            and (new_stream[0] == "=" or new_stream.isalnum())
        ):
            self.stream = self.stream[1:]
            return "&"
        if not has_semi:
            self.error("missing-semicolon-after-character-reference")
        self.stream = new_stream
        return chars

    def read_num_char_ref(self, base, num_stream) -> str:
        BASE_CHARS = "0123456789abcdef"[:base]
        if num_stream[0].lower() not in BASE_CHARS:
            self.error("absence-of-digits-in-numeric-character-reference")
            self.stream = self.stream[1:]
            return "&"
        codepoint = 0
        while True:
            if num_stream[0] in "0123456789":
                codepoint *= base
                codepoint += ord(num_stream[0]) - ord("0")
                num_stream = num_stream[1:]
            elif num_stream[0].lower() in "abcdef":
                codepoint *= base
                codepoint += ord(num_stream[0].lower()) - ord("a")
                num_stream = num_stream[1:]
            elif num_stream[0] == ";":
                num_stream = num_stream[1:]
                break
            else:
                self.error("missing-semicolon-after-character-reference")
                break
        if codepoint == 0:
            self.error("null-character-reference")
            codepoint = 0xFFFD
        elif codepoint > 0x10FFFF:
            self.error("character-reference-outside-unicode-range")
            codepoint = 0xFFFD
        elif is_surrogate(codepoint):
            self.error("surrogate-character-reference")
            codepoint = 0xFFFD
        elif is_nonchar(codepoint):
            self.error("noncharacter-character-reference")
        elif (
            codepoint == 0x0D
            or not is_ascii_whitespace(chr(codepoint))
            and is_control(codepoint)
        ):
            self.error("control-character-reference")
            if CC_MAP[codepoint] is not None:
                codepoint = CC_MAP[codepoint]
        self.stream = num_stream
        return chr(codepoint)
