from os import PRIO_USER
from typing import Any, Callable, Dict, List, Literal, Optional, Tuple, TypeVar
from dataclasses import dataclass
from enum import Enum, auto
from typing_extensions import ValuesView


class Tok(Enum):
    EOF = auto()
    SPACE = auto()
    IDENT = auto()
    FUNCTION = auto()
    ATKW = auto()
    HASH = auto()
    STR = auto()
    BADSTR = auto()
    URL = auto()
    BADURL = auto()
    DELIM = auto()
    NUMBER = auto()
    PERCENTAGE = auto()
    DIMENSION = auto()
    UNICODE_RANGE = auto()
    CDO = auto()
    CDC = auto()
    COLON = auto()
    SEMI = auto()
    COMMA = auto()
    LSQUARE = auto()
    RSQUARE = auto()
    LPAREN = auto()
    RPAREN = auto()
    LBRACE = auto()
    RBRACE = auto()


@dataclass
class Token:
    kind: Tok
    value: str
    unit: str
    num_value: float
    typeflag: Literal["id", "unrestricted", "number", "integer"]

    def __init__(self, kind: Tok):
        self.kind = kind
        self.value = ""
        self.unit = ""
        self.typeflag = "unrestricted"
        self.num_value = 0

    def __str__(self) -> str:
        match self.kind:
            case Tok.CDO:
                return "<!--"
            case Tok.CDC:
                return "-->"
            case Tok.COLON:
                return ":"
            case Tok.SEMI:
                return ";"
            case Tok.COMMA:
                return ","
            case Tok.LSQUARE:
                return "["
            case Tok.RSQUARE:
                return "]"
            case Tok.LPAREN:
                return "("
            case Tok.RPAREN:
                return ")"
            case Tok.LBRACE:
                return "{"
            case Tok.RBRACE:
                return "}"
            case Tok.SPACE:
                return " "
            case Tok.IDENT:
                return self.value
            case Tok.URL:
                return f"url({self.value})"
            case Tok.STR:
                return f'"{self.value}"'
            case Tok.DELIM:
                return self.value
            case Tok.FUNCTION | Tok.ATKW:
                return self.kind.name + "(" + self.value + ")"
            case Tok.HASH:
                return self.kind.name + "[" + self.typeflag + "](" + self.value + ")"
            case Tok.NUMBER:
                return self.value + str(
                    (self.num_value, int(self.num_value))[self.typeflag == "integer"]
                )
            case Tok.DIMENSION:
                return (
                    self.value
                    + str(
                        (self.num_value, int(self.num_value))[
                            self.typeflag == "integer"
                        ]
                    )
                    + self.unit
                )
            case Tok.PERCENTAGE:
                return self.value + str(self.num_value) + "%"
            case Tok.UNICODE_RANGE:
                return self.kind.name + "[" + self.value[0] + ", " + self.value[1] + "]"
            case _:
                return self.kind.name

    def __repr__(self) -> str:
        s = self.__str__()
        if self.kind == Tok.IDENT:
            return f"id({s})"
        if self.kind != Tok.URL:
            return f"'{s}'"
        return s


type TokenVal = Token | Function | SimpleBlock


@dataclass
class TokenStream:
    tokens: List[TokenVal]
    index: int
    marked_indexes: List[int]

    def __init__(self, tokens: List[TokenVal]):
        self.tokens = tokens
        self.index = 0
        self.marked_indexes = []

    def next_token(self) -> TokenVal:
        if self.empty():
            return Token(Tok.EOF)
        return self.tokens[self.index]

    def next(self) -> Token:
        if self.empty():
            return Token(Tok.EOF)
        out = self.tokens[self.index]
        assert isinstance(out, Token)
        return out

    def empty(self) -> bool:
        return self.index >= len(self.tokens)

    def consume_token(self) -> TokenVal:
        token = self.next_token()
        self.index += 1
        return token

    def discard_token(self):
        if not self.empty():
            self.index += 1

    def mark(self):
        self.marked_indexes.append(self.index)

    def restore_mark(self):
        self.index = self.marked_indexes.pop()

    def discard_mark(self):
        self.marked_indexes.pop()

    def discard_whitespace(self) -> bool:
        nt = self.next_token()
        out = False
        while isinstance(nt, Token) and nt.kind == Tok.SPACE:
            self.discard_token()
            out = True
            nt = self.next_token()
        return out

    def process(
        self,
        process_dict: Dict[Tok, Callable[["TokenStream"], Any]],
        other: Callable[["TokenStream"], Any],
    ):
        while True:
            if self.empty():
                if Tok.EOF in process_dict.keys():
                    return process_dict[Tok.EOF](self)
                return other(self)
            nt = self.next_token()
            if isinstance(nt, Function) or isinstance(nt, SimpleBlock):
                return
            action = other
            if nt.kind in process_dict.keys():
                action = process_dict[nt.kind]
            res = action(self)
            if res is not None:
                return res


def is_surrogate(c: str) -> bool:
    return c == "\0"  # TODO:


def is_ident(c: str) -> bool:
    return is_ident_start(c) or c in "-0123456789"


def is_ident_start(c: str) -> bool:
    def non_ascii_ident(c: str) -> bool:
        _ = c
        return False

    return (
        c in "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
        or non_ascii_ident(c)
    )


def is_escape(s: str) -> bool:
    if len(s) < 2:
        return False
    if s[0] != "\\":
        return False
    return s[1] != "\n"


def is_start_ident_sequence(s: str) -> bool:
    if len(s) == 0:
        return False
    n = max(3, len(s))
    s = s[:n]
    while len(s) < 3:
        s += "\0"
    if s[0] == "-":
        return is_ident_start(s[1]) or is_escape(s[1:])
    return is_ident_start(s[0]) or is_escape(s[:2])


def is_start_unicode_range(s: str) -> bool:
    if len(s) == 0:
        return False
    n = max(3, len(s))
    s = s[:n]
    while len(s) < 3:
        s += "\0"
    if s[0] not in "uU" or s[1] != "+":
        return False
    return s[2] in "?0123456789abcdefABCDEF"


def filter_code_points(s: str) -> str:
    out = ""
    seen_cr = False
    for c in s:
        if seen_cr and c != "\n":
            out += "\n"
        seen_cr = False
        if is_surrogate(c):
            out += "\ufffd"
        elif c == "\f":
            out += "\n"
        elif c == "\r":
            seen_cr = True
        else:
            out += c
    return out


def consume_comments(s: str) -> str:
    while s.startswith("/*"):
        s = s[2:]
        while not s.startswith("*/"):
            if len(s) == 0:
                parse_error(s, "eof in comment")
                return ""
            s = s[1:]
        s = s[2:]
    return s


def consume_escape(s: str) -> Tuple[str, str]:
    if len(s) == 0:
        parse_error(s, "eof in escape")
        return "\ufffd", ""
    c, s = s[0], s[1:]
    if c in "0123456789abcdefABCDEF":

        def hexdigit(d: str) -> int:
            if d in "0123456789":
                return ord(d) - ord("0")
            if d in "abcdef":
                return ord(d) - ord("a") + 10
            if d in "ABCDEF":
                return ord(d) - ord("A") + 10
            return 0

        codepoint = hexdigit(c)
        while len(s) > 0 and s[0] in "0123456789abcdefABCDEF":
            codepoint = codepoint * 16 + hexdigit(s[0])
            s = s[1:]
        if len(s) > 0 and s[0].isspace():
            s = s[1:]
        c = chr(codepoint)
        if is_surrogate(c):
            return "\ufffd", s
    return c, s


def consume_string_token(s: str, ending: str = "") -> Tuple[Token, str]:
    t = Token(Tok.STR)
    t.value = ""
    assert len(s) > 0
    if len(ending) != 1:
        ending, s = s[0], s[1:]
    while True:
        if len(s) == 0:
            parse_error(s, "eof in string")
            return t, s
        c, s = s[0], s[1:]
        if c == ending:
            return t, s
        if c == "\n":
            parse_error(s, "newline in string")
            return Token(Tok.BADSTR), c + s
        if c == "\\":
            if len(s) == 0:
                pass
            elif s.startswith("\n"):
                s = s[1:]
            else:
                c, s = consume_escape(s)
                t.value += c
        else:
            t.value += c


def consume_ident_sequence(s: str) -> Tuple[str, str]:
    res = ""
    while len(s) > 0:
        c, s = s[0], s[1:]
        if is_ident(c):
            res += c
        elif is_escape(c + s):
            c, s = consume_escape(s)
            res += c
        else:
            return res, c + s
    return res, s


def consume_number(s: str) -> Tuple[Token, str]:
    num = Token(Tok.NUMBER)
    num.typeflag = "integer"
    if s.startswith("+") or s.startswith("-"):
        num.value, s = s[0], s[1:]
    num_part = ""
    exp_part = ""
    while len(s) > 0 and s[0] in "0123456789":
        num_part += s[0]
        s = s[1:]
    if len(s) > 1 and s[0] == "." and s[1] in "0123456789":
        num_part += "."
        s = s[1:]
        num.typeflag = "number"
        while len(s) > 0 and s[0] in "0123456789":
            num_part += s[0]
            s = s[1:]
    if (
        len(s) > 1
        and s[0] in "eE"
        and (
            s[1] in "0123456789" or s[1] in "+-" and len(s) > 2 and s[2] in "0123456789"
        )
    ):
        s = s[1:]
        num.typeflag = "number"
        if s[0] in "+-":
            exp_part += s[0]
            s = s[1:]
        while len(s) > 0 and s[0] in "0123456789":
            exp_part += s[0]
            s = s[1:]
    value = float(num_part)
    if exp_part != "":
        exponent = int(exp_part)
        value *= 10**exponent
    num.num_value = value
    return num, s


def consume_numeric_token(s: str) -> Tuple[Token, str]:
    result, s = consume_number(s)
    if is_start_ident_sequence(s):
        result.kind = Tok.DIMENSION
        result.unit, s = consume_ident_sequence(s)
        return result, s
    if s.startswith("%"):
        result.kind = Tok.PERCENTAGE
        return result, s[1:]
    return result, s


def consume_ident_like_token(s: str) -> Tuple[Token, str]:
    out, s = consume_ident_sequence(s)
    if out == "url" and s.startswith("("):
        s = s[1:]
        while len(s) > 1 and s[0].isspace() and s[1].isspace():
            s = s[2:]
        if (
            len(s) > 0
            and s[0] in "\"'"
            or s[0].isspace()
            and len(s) > 1
            and s[1] in "\"'"
        ):
            t = Token(Tok.FUNCTION)
            t.value = s
            return t, s
        return consume_url_token(s)
    if s.startswith("("):
        t = Token(Tok.FUNCTION)
        t.value = out
        return t, s[1:]
    t = Token(Tok.IDENT)
    t.value = out
    return t, s


def consume_url_token(s: str) -> Tuple[Token, str]:
    t = Token(Tok.URL)
    t.value = ""
    while len(s) > 0 and s[0].isspace():
        s = s[1:]
    while True:
        if len(s) == 0:
            parse_error(s, "eof in url")
            return t, s
        c, s = s[0], s[1:]
        if c == ")":
            return t, s
        if c.isspace():
            while len(s) > 0 and s[0].isspace():
                s = s[1:]
            if len(s) == 0:
                parse_error(s, "eof in url")
                return t, s
            if s[0] == ")":
                return t, s[1:]
            s = consume_remnants_of_bad_url_token(s)
            return Token(Tok.BADURL), s
        if not c.isprintable() or c in "\"'(":
            parse_error(s, "invalid char in url", c)
            s = consume_remnants_of_bad_url_token(s)
            return Token(Tok.BADURL), s
        if c == "\\":
            if is_escape(c + s):
                c, s = consume_escape(s)
            else:
                parse_error(s, "invalid char in url")
                s = consume_remnants_of_bad_url_token(s)
                return Token(Tok.BADURL), s
        t.value += c


def consume_remnants_of_bad_url_token(s: str) -> str:
    while len(s) > 0:
        if s[0] == ")":
            return s[1:]
        if is_escape(s):
            _, s = consume_escape(s[1:])
        else:
            s = s[1:]
    return s


def consume_unicode_range_token(s: str) -> Tuple[Token, str]:
    _ = s  # TODO:
    assert False, "TODO"


def consume_token(s: str, unicode_ranges_allowed: bool = False) -> Tuple[Token, str]:
    s = consume_comments(s)
    if len(s) == 0:
        return Token(Tok.EOF), s
    c, s = s[0], s[1:]
    if c.isspace():
        while c.isspace():
            if len(s) == 0:
                return Token(Tok.SPACE), s
            c, s = s[0], s[1:]
        return Token(Tok.SPACE), c + s
    elif c == '"' or c == "'":
        return consume_string_token(s, c)
    elif c == "#":
        if len(s) > 0 and is_ident(s[0]) or (len(s) > 1 and is_escape(s[:2])):
            t = Token(Tok.HASH)
            if is_start_ident_sequence(s):
                t.typeflag = "id"
            ident, s = consume_ident_sequence(s)
            t.value = ident
            return t, s
        t = Token(Tok.DELIM)
        t.value = c
        return t, s
    elif c == "+":
        if len(s) > 0 and s[0] in "0123456789":
            return consume_numeric_token(c + s)
        t = Token(Tok.DELIM)
        t.value = c
        return t, s
    elif c == "-":
        if len(s) > 0 and s[0] in "0123456789":
            return consume_numeric_token(c + s)
        if s.startswith("->"):
            return Token(Tok.CDC), s[2:]
        if is_start_ident_sequence(s):
            return consume_ident_like_token(c + s)
        t = Token(Tok.DELIM)
        t.value = c
        return t, s
    elif c == ".":
        if len(s) > 0 and s[0] in "0123456789":
            return consume_numeric_token(c + s)
        t = Token(Tok.DELIM)
        t.value = c
        return t, s
    elif c == "\\":
        if is_escape(s):
            return consume_ident_like_token(c + s)
        parse_error(s, "backslash in input")
        t = Token(Tok.DELIM)
        t.value = c
        return t, s
    elif c == "<":
        if s.startswith("!--"):
            return Token(Tok.CDO), s[3:]
        t = Token(Tok.DELIM)
        t.value = c
        return t, s
    elif c == "@":
        if is_start_ident_sequence(s):
            ident, s = consume_ident_sequence(s)
            t = Token(Tok.ATKW)
            t.value = ident
            return t, s
        t = Token(Tok.DELIM)
        t.value = c
        return t, s
    elif c == "(":
        return Token(Tok.LPAREN), s
    elif c == ")":
        return Token(Tok.RPAREN), s
    elif c == "{":
        return Token(Tok.LBRACE), s
    elif c == "}":
        return Token(Tok.RBRACE), s
    elif c == "[":
        return Token(Tok.LSQUARE), s
    elif c == "]":
        return Token(Tok.RSQUARE), s
    elif c == ":":
        return Token(Tok.COLON), s
    elif c == ",":
        return Token(Tok.COMMA), s
    elif c == ";":
        return Token(Tok.SEMI), s
    elif c in "0123456789":
        return consume_numeric_token(c + s)
    elif c in "uU" and unicode_ranges_allowed and is_start_unicode_range(c + s):
        return consume_unicode_range_token(c + s)
    elif is_ident_start(c):
        return consume_ident_like_token(c + s)
    else:
        t = Token(Tok.DELIM)
        t.value = c
        return t, s


def parse_error(s: str, *args):
    print("parse error...", s[:40], args)


def tokenize(s: str) -> List[TokenVal]:
    out = []
    while True:
        t, s = consume_token(s)
        if t.kind == Tok.EOF:
            return out
        out.append(t)


def normalize_input(i):
    if isinstance(i, TokenStream):
        return i
    if isinstance(i, list):
        for t in i:
            assert (
                isinstance(t, Token)
                or isinstance(t, SimpleBlock)
                or isinstance(t, Function)
            )
        return TokenStream(i)
    if isinstance(i, str):
        s = filter_code_points(i)
        return TokenStream(tokenize(s))
    assert False, "unreachable"


@dataclass
class Declaration:
    name: str
    value: List[TokenVal]
    important: bool

    def __init__(self):
        self.name = ""
        self.value = []
        self.important = False

    def is_valid(self) -> bool:
        return True

    def __repr__(self) -> str:
        return f"{self.name}: {self.value}" + ("", " !important")[self.important]

    def __str__(self) -> str:
        val = "".join(str(s) for s in self.value) + ("", " !important")[self.important]
        return f"{self.name}:{val};"


type StyleRule = QualifiedRule | NestedDeclarationRule
type CSSRule = AtRule | StyleRule


@dataclass
class AtRule:
    name: str
    prelude: List[TokenVal]
    declarations: List[Declaration]
    child_rules: List[CSSRule]
    children: list

    def __init__(self, name: str):
        self.name = name
        self.prelude = []
        self.declarations = []
        self.child_rules = []
        self.children = []

    def is_valid(self) -> bool:
        return True

    def set_children(self, a: list):
        # TODO:
        for c in a:
            if isinstance(c, list):
                self.children += c
            else:
                self.children.append(c)

    def __str__(self):
        a = "".join(str(s) for s in self.prelude)
        b = "\n".join(str(s) for s in self.children)
        return f"@{self.name} {a} {{\n{b}\n}}"


@dataclass
class Selector:
    pass


@dataclass
class Selectors:
    inner: List[Selector]

    def __str__(self) -> str:
        selstr = ""
        for i, s in enumerate(self.inner):
            if i != 0:
                selstr += ", "
            selstr += str(s)
        return selstr

    def __repr__(self):
        return f"'{self}'"


@dataclass
class QualifiedRule:
    selectors: Selectors
    declarations: List[Declaration]
    child_rules: List[CSSRule]

    def __init__(self):
        self.selectors = Selectors([])
        self.declarations = []
        self.child_rules = []

    def is_valid(self) -> bool:
        return True

    def set_children(self, a: list):
        _ = a
        pass

    def __str__(self) -> str:
        out = f"{self.selectors} {{\n"
        for d in self.declarations:
            out += f"  {d}\n"
        for cr in self.child_rules:
            out += str(cr) + "\n"
        out += "}"
        return out


END_TOKEN = {
    Tok.LSQUARE: Tok.RSQUARE,
    Tok.LPAREN: Tok.RPAREN,
    Tok.LBRACE: Tok.RBRACE,
}


@dataclass
class NestedDeclarationRule:
    declarations: List[Declaration]

    def __str__(self) -> str:
        return "\n".join(str(s) for s in self.declarations)


@dataclass
class SimpleBlock:
    token: Tok
    value: List[TokenVal]

    def __str__(self) -> str:
        a = "".join(str(s) for s in self.value)
        return f"{Token(self.token)}{a}{Token(END_TOKEN[self.token])}"


@dataclass
class Function:
    name: str
    value: List[TokenVal]

    def __str__(self) -> str:
        # TODO:
        return f"{self.name}({' '.join(str(s) for s in self.value)})"


@dataclass
class Nothing:
    pass


@dataclass
class InvalidRuleError:
    pass


def consume_simple_block(stream: TokenStream) -> SimpleBlock:
    nt = stream.next_token()
    assert isinstance(nt, Token) and nt.kind in [Tok.LSQUARE, Tok.LPAREN, Tok.LBRACE]
    end_token = END_TOKEN[nt.kind]
    block = SimpleBlock(nt.kind, [])
    stream.discard_token()
    out = stream.process(
        {
            Tok.EOF: lambda s: (s.discard_token(), block)[1],
            end_token: lambda s: (s.discard_token(), block)[1],
        },
        lambda s: block.value.append(consume_component_value(s)),
    )
    assert isinstance(out, SimpleBlock)
    return out


def consume_function(stream: TokenStream) -> Function:
    nt = stream.next_token()
    assert isinstance(nt, Token) and nt.kind == Tok.FUNCTION
    function = Function(nt.value, [])
    stream.discard_token()
    while True:
        match stream.next().kind:
            case Tok.EOF | Tok.RPAREN:
                stream.discard_token()
                return function
            case _:
                function.value.append(consume_component_value(stream))


def consume_component_value(stream: TokenStream) -> TokenVal:
    match stream.next().kind:
        case Tok.LSQUARE | Tok.LPAREN | Tok.LBRACE:
            return consume_simple_block(stream)
        case Tok.FUNCTION:
            return consume_function(stream)
        case _:
            return stream.consume_token()


def consume_at_rule(stream: TokenStream, nested: bool = False) -> Optional[AtRule]:
    nt = stream.next()
    assert nt.kind == Tok.ATKW
    rule = AtRule(nt.value)
    stream.consume_token()
    while True:
        match stream.next().kind:
            case Tok.SEMI | Tok.EOF:
                stream.discard_token()
                return rule if rule.is_valid() else None
            case Tok.RBRACE:
                if nested:
                    return rule if rule.is_valid() else None
                rule.prelude.append(stream.consume_token())
            case Tok.LBRACE:
                rule.set_children(consume_block(stream))
                return rule if rule.is_valid() else None
            case _:
                rule.prelude.append(consume_component_value(stream))


def consume_remnants_of_bad_declaration(stream: TokenStream, nested: bool) -> None:
    def rbrace(s: TokenStream):
        if nested:
            return Nothing()
        s.discard_token()

    stream.process(
        {
            Tok.EOF: lambda s: (s.discard_token(), Nothing())[1],
            Tok.SEMI: lambda s: (s.discard_token(), Nothing())[1],
            Tok.RBRACE: rbrace,
        },
        lambda s: (consume_component_value(s), None)[1],
    )


type BlockType = List[List[Declaration] | CSSRule]


def consume_block(stream: TokenStream) -> BlockType:
    assert stream.next().kind == Tok.LBRACE
    stream.discard_token()
    rules = consume_block_contents(stream)
    stream.discard_token()
    return rules


def consume_list_of_component_values(
    stream: TokenStream, nested: bool = False, stop_token: Optional[Tok] = None
) -> List[TokenVal]:
    values: List[TokenVal] = []
    while True:
        match stream.next().kind:
            case Tok.EOF:
                return values
            case x if x == stop_token:
                return values
            case Tok.RBRACE:
                if nested:
                    return values
                parse_error("rbrace in value")
                values.append(stream.consume_token())
            case _:
                values.append(consume_component_value(stream))


def consume_declaration(
    stream: TokenStream, nested: bool = False
) -> Optional[Declaration]:
    decl = Declaration()
    nt = stream.next()
    if nt.kind == Tok.IDENT:
        stream.consume_token()
        decl.name = nt.value
    else:
        consume_remnants_of_bad_declaration(stream, nested)
        return
    stream.discard_whitespace()
    nt = stream.next_token()
    if isinstance(nt, Token) and nt.kind == Tok.COLON:
        stream.discard_token()
    else:
        consume_remnants_of_bad_declaration(stream, nested)
        return
    value = consume_list_of_component_values(stream, nested, Tok.SEMI)
    assert isinstance(value, list)
    decl.value = value

    # important
    end = decl.value
    while len(end) > 0 and isinstance(end[-1], Token) and end[-1].kind == Tok.SPACE:
        end = end[:-1]
    if (
        len(end) >= 1
        and isinstance(end[-1], Token)
        and end[-1].kind == Tok.IDENT
        and end[-1].value.lower() == "important"
    ):
        end_o = end
        end_o = end_o[:-1]
        while (
            len(end_o) > 0
            and isinstance(end_o[-1], Token)
            and end_o[-1].kind == Tok.SPACE
        ):
            end_o = end_o[:-1]
        if (
            len(end_o) >= 1
            and isinstance(end_o[-1], Token)
            and end_o[-1].kind == Tok.DELIM
            and end_o[-1].value == "!"
        ):
            decl.important = True
            end = end_o[:-1]
            while (
                len(end) > 0
                and isinstance(end[-1], Token)
                and end[-1].kind == Tok.SPACE
            ):
                end = end[:-1]
    decl.value = end

    # if decl.name.startswith("--"):
    #   set decl’s original text to the segment of the original source text string corresponding to the tokens of decl’s value.

    has_non_ws = 0
    has_block = False
    for d in decl.value:
        if isinstance(d, Token) and d.kind == Tok.SPACE:
            continue
        has_non_ws += 1
        if isinstance(d, SimpleBlock):
            has_block = True
    if has_block and has_non_ws > 1:
        return None

    if decl.name.lower() == "unicode-range":
        print("unicode range: TODO")
        return None

    if decl.is_valid():
        return decl
    return None


def consume_block_contents(stream: TokenStream) -> BlockType:
    rules = []
    decls = []
    while True:
        match stream.next().kind:
            case Tok.SPACE | Tok.SEMI:
                stream.discard_token()
            case Tok.EOF | Tok.RBRACE:
                if len(decls) != 0:
                    rules.append(decls)
                return rules
            case Tok.ATKW:
                if len(decls) != 0:
                    rules.append(decls)
                    decls = []
                r = consume_at_rule(stream, True)
                if r is not None:
                    rules.append(r)
            case _:
                stream.mark()
                d = consume_declaration(stream, True)
                if d is not None:
                    decls.append(d)
                    stream.discard_mark()
                    continue
                stream.restore_mark()
                r = consume_qualified_rule(stream, Tok.SEMI, True)
                if r is not None:
                    if len(decls) != 0:
                        rules.append(decls)
                        decls = []
                    if not isinstance(r, InvalidRuleError):
                        rules.append(r)


class SelectorParseError(Exception):
    pass

NESTED = False

def consume_qualified_rule(
    stream: TokenStream, stop_token: Optional[Tok] = None, nested: bool = False
) -> Optional[QualifiedRule | InvalidRuleError]:
    global NESTED
    NESTED = nested
    rule = QualifiedRule()
    prelude: List[TokenVal] = []
    while True:
        match stream.next().kind:
            case Tok.EOF:
                parse_error("eof in rule")
                return
            case x if x == stop_token:
                parse_error("stop token in rule", stop_token)
                return
            case Tok.RBRACE:
                parse_error("rbrace in rule")
                if nested:
                    return
                prelude.append(stream.consume_token())
            case Tok.LBRACE:
                p = prelude
                while len(p) > 0 and isinstance(p[0], Token) and p[0].kind == Tok.SPACE:
                    p = p[1:]
                if len(p) >= 2 and isinstance(p[0], Token) and isinstance(p[1], Token):
                    if (
                        p[0].kind == Tok.IDENT
                        and p[0].value.startswith("--")
                        and p[1].kind == Tok.COLON
                    ):
                        if nested:
                            consume_remnants_of_bad_declaration(stream, True)
                        else:
                            consume_block(stream)
                        return
                child_rules = consume_block(stream)
                rule.child_rules = []
                if len(child_rules) > 0 and isinstance(child_rules[0], list):
                    rule.declarations, child_rules = child_rules[0], child_rules[1:]
                for c in child_rules:
                    if isinstance(c, list):
                        rule.child_rules.append(NestedDeclarationRule(c))
                    else:
                        rule.child_rules.append(c)
                try:
                    rule.selectors.inner = parse_selector_list(TokenStream(prelude))
                    return rule
                except SelectorParseError:
                    return InvalidRuleError()
            case _:
                prelude.append(consume_component_value(stream))


def consume_stylesheet(stream: TokenStream) -> List[CSSRule]:
    rules = []
    while True:
        match stream.next().kind:
            case Tok.EOF:
                return rules
            case Tok.SPACE | Tok.CDO | Tok.CDC:
                stream.discard_token()
            case Tok.ATKW:
                r = consume_at_rule(stream)
                if r is not None:
                    rules.append(r)
            case _:
                r = consume_qualified_rule(stream)
                if r is not None:
                    rules.append(r)


def main():
    from sys import argv

    fname = "/home/clement/dev/web/public/styles.css"
    if len(argv) > 1:
        fname = argv[1]
    i = ""
    with open(fname) as f:
        i = f.read()
    tokenstream = normalize_input(i)
    out = consume_stylesheet(tokenstream)
    import pprint

    pprint.pprint(out)
    # for r in out:
    #     print(r)


T = TypeVar("T")


def parse_commaseplist(stream: TokenStream, fn: Callable[[TokenStream], T]) -> List[T]:
    stream.discard_whitespace()
    a = parse_opt(stream, fn)
    if a is None:
        return []
    out = [a]
    while True:
        stream.discard_whitespace()
        nt = stream.next_token()
        if not isinstance(nt, Token) or nt.kind != Tok.COMMA:
            break
        stream.discard_token()
        stream.discard_whitespace()
        out.append(fn(stream))
    return out


def parse_opt(stream: TokenStream, fn: Callable[[TokenStream], T]) -> Optional[T]:
    stream.mark()
    try:
        out = fn(stream)
        stream.discard_mark()
        return out
    except SelectorParseError:
        stream.restore_mark()
        return None


def parse_many(stream: TokenStream, fn: Callable[[TokenStream], T]) -> List[T]:
    out: List[T] = []
    while True:
        val = parse_opt(stream, fn)
        if val is None:
            return out
        out.append(val)


def parse_alt(stream: TokenStream, *args):
    assert len(args) > 0
    for a in args:
        stream.mark()
        val = parse_opt(stream, a)
        if val is not None:
            stream.discard_mark()
            return val
        stream.restore_mark()
    raise SelectorParseError


def parse_seq(stream: TokenStream, *args):
    assert len(args) > 0
    out = []
    for a in args:
        out.append(a(stream))
    return out


def parse_selector_list(stream: TokenStream) -> List[Selector]:
    return parse_complex_selector_list(stream)


def parse_complex_selector_list(stream: TokenStream) -> List[Selector]:
    return parse_commaseplist(stream, parse_complex_selector)


def parse_complex_real_selector_list(stream: TokenStream) -> List[Selector]:
    return parse_commaseplist(stream, parse_complex_real_selector)


def parse_compound_selector_list(stream: TokenStream) -> List[Selector]:
    return parse_commaseplist(stream, parse_compound_selector)


def parse_simple_selector_list(stream: TokenStream) -> List[Selector]:
    return parse_commaseplist(stream, parse_simple_selector)


def parse_relative_selector_list(stream: TokenStream) -> List[Selector]:
    return parse_commaseplist(stream, parse_relative_selector)


def parse_relative_real_selector_list(stream: TokenStream) -> List[Selector]:
    return parse_commaseplist(stream, parse_relative_real_selector)


@dataclass
class AmpSelector(Selector):
    def __str__(self) -> str:
        return "&"


@dataclass
class ComplexSelector(Selector):
    selectors: List[Selector]
    combinators: List[str]

    def __str__(self) -> str:
        assert len(self.combinators) == len(self.selectors) - 1
        assert len(self.selectors) >= 1
        out = str(self.selectors[0])
        for i in range(len(self.combinators)):
            out += f" {self.combinators[i]} {self.selectors[i + 1]}"
        return out


def parse_complex_selector(stream: TokenStream) -> Selector:
    lst = [parse_complex_selector_unit(stream)]
    cmbs = []
    while True:
        had_ws = stream.discard_whitespace()
        combinator = parse_opt(stream, parse_combinator)
        if combinator is not None:
            stream.discard_whitespace()
            cmbs.append(combinator)
            lst.append(parse_complex_selector_unit(stream))
        else:
            n = parse_opt(stream, parse_complex_selector_unit)
            if n is None:
                break
            if not had_ws:
                raise SelectorParseError
            cmbs.append(" ")
            lst.append(n)
    if len(lst) == 1:
        return lst[0]
    return ComplexSelector(lst, cmbs)


@dataclass
class CompoundSelector(Selector):
    subsels: List[Selector]

    def __str__(self) -> str:
        return "".join(str(s) for s in self.subsels)


def parse_complex_selector_unit(stream: TokenStream) -> Selector:  # NOSPACE
    compound_selector = parse_opt(stream, parse_compound_selector)
    pseudo_compound_selector = parse_many(stream, parse_pseudo_compound_selector)
    lst: List[Selector] = []
    for s in pseudo_compound_selector:
        lst += s.subsels
    if compound_selector is not None:
        if isinstance(compound_selector, CompoundSelector):
            lst = compound_selector.subsels + lst
        else:
            lst = [compound_selector] + lst
    if len(lst) == 0:
        raise SelectorParseError
    if len(lst) == 1:
        return lst[0]
    return CompoundSelector(lst)


def parse_complex_real_selector(stream: TokenStream) -> Selector:
    lst = [parse_compound_selector(stream)]
    cmbs = []
    while True:
        had_ws = stream.discard_whitespace()
        combinator = parse_opt(stream, parse_combinator)
        if combinator is not None:
            stream.discard_whitespace()
            cmbs.append(combinator)
            lst.append(parse_compound_selector(stream))
        else:
            if not had_ws:
                raise SelectorParseError
            unit = parse_opt(stream, parse_compound_selector)
            if unit is None:
                break
            cmbs.append(" ")
            lst.append(unit)
    if len(lst) == 1:
        return lst[0]
    return ComplexSelector(lst, cmbs)


def parse_compound_selector(stream: TokenStream) -> Selector:
    ty = parse_opt(stream, parse_type_selector)
    sc = parse_many(stream, parse_subclass_selector)
    if ty is not None:
        sc = [ty] + sc
    if len(sc) == 0:
        raise SelectorParseError
    if len(sc) == 1:
        return sc[0]
    return CompoundSelector(sc)


def parse_simple_selector(stream: TokenStream) -> Selector:
    return parse_alt(stream, parse_type_selector, parse_subclass_selector)


@dataclass
class RelativeSelector(Selector):
    comb: Optional[str]
    subsel: Selector

    def __str__(self) -> str:
        if self.comb is not None:
            return f"{self.comb} {self.subsel}"
        return str(self.subsel)


def parse_relative_selector(stream: TokenStream) -> RelativeSelector:
    combinator = parse_opt(stream, parse_combinator)
    stream.discard_whitespace()
    selector = parse_complex_selector(stream)
    return RelativeSelector(combinator, selector)


def parse_relative_real_selector(stream: TokenStream) -> RelativeSelector:
    combinator = parse_opt(stream, parse_combinator)
    stream.discard_whitespace()
    selector = parse_complex_real_selector(stream)
    return RelativeSelector(combinator, selector)


def parse_combinator(stream: TokenStream) -> str:  # NOSPACE
    stream.discard_whitespace()
    nt = stream.next_token()
    if not isinstance(nt, Token) or nt.kind != Tok.DELIM:
        raise SelectorParseError
    match nt.value:
        case ">" | "+" | "~":
            stream.discard_token()
            return nt.value
        case "|":
            stream.discard_token()
            nt = stream.next_token()
            if isinstance(nt, Token) and nt.kind == Tok.DELIM and nt.value == "|":
                stream.discard_token()
                return "||"
            return "|"
        case _:
            raise SelectorParseError


@dataclass
class PseudoElementSelector(Selector):
    val: str | Function

    def __str__(self) -> str:
        return "::" + str(self.val)


@dataclass
class PseudoClassSelector(Selector):
    val: str | Function

    def __str__(self) -> str:
        return ":" + str(self.val)


def parse_pseudo_compound_selector(stream: TokenStream) -> CompoundSelector:
    pseudoelt = parse_pseudo_element_selector(stream)
    # stream.discard_whitespace() # TODO: should it ?
    pseudoclass = parse_many(stream, parse_pseudo_class_selector)
    lst: List[Selector] = [s for s in pseudoclass]
    lst = [pseudoelt] + lst
    return CompoundSelector(lst)


def parse_pseudo_element_selector(stream: TokenStream) -> PseudoElementSelector:
    nt = stream.next_token()
    if not isinstance(nt, Token) or nt.kind != Tok.COLON:
        raise SelectorParseError
    stream.discard_token()
    pc = parse_pseudo_class_selector(stream)
    return PseudoElementSelector(pc.val)


def parse_pseudo_class_selector(stream: TokenStream) -> PseudoClassSelector:
    nt = stream.next_token()
    if not isinstance(nt, Token) or nt.kind != Tok.COLON:
        raise SelectorParseError
    stream.discard_token()
    nt = stream.next_token()
    if not (isinstance(nt, Token) and nt.kind == Tok.IDENT or isinstance(nt, Function)):
        raise SelectorParseError
    stream.discard_token()
    if isinstance(nt, Token):
        nt = nt.value
    return PseudoClassSelector(nt)


@dataclass
class IdSelector(Selector):
    name: str

    def __str__(self) -> str:
        return f"#{self.name}"


def parse_id_selector(stream: TokenStream) -> IdSelector:
    nt = stream.next_token()
    if not isinstance(nt, Token) or nt.kind != Tok.HASH or nt.typeflag != "id":
        raise SelectorParseError
    stream.discard_token()
    return IdSelector(nt.value)


@dataclass
class TypeSelector(Selector):
    namespace: Optional[str]
    typename: str

    def __str__(self) -> str:
        if self.namespace is not None:
            return f"{self.namespace}|{self.typename}"
        return self.typename


def parse_type_selector(stream: TokenStream) -> TypeSelector:
    pfx = parse_opt(stream, parse_ns_prefix)
    # pfx should be valid...
    nt = stream.next_token()
    if not isinstance(nt, Token) or not (
        nt.kind == Tok.IDENT or nt.kind == Tok.DELIM and nt.value == "*"
    ):
        raise SelectorParseError
    stream.discard_token()
    return TypeSelector(pfx, nt.value)


def parse_subclass_selector(stream: TokenStream) -> Selector:
    return parse_alt(
        stream,
        parse_id_selector,
        parse_class_selector,
        parse_attribute_selector,
        parse_pseudo_class_selector,
        parse_amp_selector,
    )

def parse_amp_selector(stream: TokenStream) -> Selector:
    if not NESTED:
        raise SelectorParseError
    t = expect_and_consume_token(stream, Tok.DELIM).value
    if t != "&":
        raise SelectorParseError
    return AmpSelector()


def parse_ns_prefix(stream: TokenStream) -> str:
    nt = stream.next_token()
    if not isinstance(nt, Token):
        raise SelectorParseError
    out = ""
    if nt.kind == Tok.IDENT or nt.kind == Tok.DELIM and nt.value == "*":
        out = nt.value
        stream.discard_token()
        nt = stream.next_token()
    if not isinstance(nt, Token) or nt.kind != Tok.DELIM or nt.value != "|":
        raise SelectorParseError
    stream.discard_token()
    return out


@dataclass
class ClassSelector(Selector):
    classname: str

    def __str__(self) -> str:
        return f".{self.classname}"


def parse_class_selector(stream: TokenStream) -> ClassSelector:  # NOSPACE
    nt = stream.next_token()
    if not isinstance(nt, Token) or nt.kind != Tok.DELIM or nt.value != ".":
        raise SelectorParseError
    stream.discard_token()
    nt = stream.next_token()
    if not isinstance(nt, Token) or nt.kind != Tok.IDENT:
        raise SelectorParseError
    stream.discard_token()
    return ClassSelector(nt.value)


def expect_and_consume_token(stream: TokenStream, *kinds: Tok) -> Token:
    nt = stream.next_token()
    if not isinstance(nt, Token) or nt.kind not in kinds:
        raise SelectorParseError
    stream.discard_token()
    return nt


def parse_wq_name(stream: TokenStream) -> str:
    pfx = parse_opt(stream, parse_ns_prefix)
    # pfx should be valid...
    nt = expect_and_consume_token(stream, Tok.IDENT)
    return f"{pfx}|{nt.value}" if pfx is not None else nt.value


@dataclass
class AttributeSelector(Selector):
    name: str
    matcher: Optional[Tuple[str, str, str]]

    def __str__(self) -> str:
        out = f"[{self.name}"
        if self.matcher is None:
            return out
        out += f" {self.matcher[0]} {self.matcher[1]}"
        if self.matcher[2] != "":
            out += " " + self.matcher[2]
        return out + "]"


def parse_attribute_selector(stream: TokenStream) -> Selector:
    expect_and_consume_token(stream, Tok.LSQUARE)
    stream.discard_whitespace()
    name = parse_wq_name(stream)
    stream.discard_whitespace()
    am = parse_opt(stream, parse_attr_matcher)
    if am is None:
        expect_and_consume_token(stream, Tok.RSQUARE)
        return AttributeSelector(name, None)
    stream.discard_whitespace()
    val = expect_and_consume_token(stream, Tok.IDENT, Tok.STR)
    stream.discard_whitespace()
    mod = parse_opt(stream, parse_attr_modifier)
    if mod is not None:
        stream.discard_whitespace()
    else:
        mod = ""
    expect_and_consume_token(stream, Tok.RSQUARE)
    return AttributeSelector(name, (am, val.value, mod))


def parse_attr_matcher(stream: TokenStream) -> str:  # NOSPACE
    t = expect_and_consume_token(stream, Tok.DELIM).value
    if t not in "~|^$*":
        raise SelectorParseError
    eq = expect_and_consume_token(stream, Tok.DELIM).value
    if eq != "=":
        raise SelectorParseError
    return t + "="


def parse_attr_modifier(stream: TokenStream) -> str:  # NOSPACE
    t = expect_and_consume_token(stream, Tok.IDENT).value
    if t != "i" and t != "s":
        raise SelectorParseError
    return t


if __name__ == "__main__":
    main()
