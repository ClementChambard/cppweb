from dataclasses import dataclass
from enum import Enum, auto
from typing import Dict, List

class Tok(Enum):
    DOCTYPE = auto()
    START_TAG = auto()
    START_COMPONENT = auto()
    END_TAG = auto()
    END_COMPONENT = auto()
    COMMENT = auto()
    CHARACTER = auto()
    PLACEHOLDER = auto()
    EOF = auto()

@dataclass
class DoctypeData:
    name: str | None
    public_ident: str | None
    system_ident: str | None
    force_quirks: bool

    def __str__(self) -> str:
        if self.force_quirks:
            return "<!DOCTYPE ... bogus ...>"
        s = "<!DOCTYPE"
        if self.name is not None:
            s += " " + self.name
        if self.public_ident is not None:
            s += ' PUBLIC "' + self.public_ident + '"'
            if self.system_ident is not None:
                s += ' "' + self.system_ident + '"'
        elif self.system_ident is not None:
            s += ' SYSTEM "' + self.system_ident + '"'
        return s + ">"

@dataclass
class PlaceholderData:
    name: str
    is_optional: bool
    replace_if_true: List["Token"] | None
    replace_if_false: List["Token"] | None

    def __str__(self) -> str:
        s = "{{" + self.name 
        if self.is_optional:
            s += "?"
        if self.replace_if_true is not None:
            s += "{{"
            for c in self.replace_if_true:
                s += str(c)
            s += "}}"
        if self.replace_if_false is not None:
            s += "?{{"
            for c in self.replace_if_false:
                s += str(c)
            s += "}}"
        return s + "}}"

type AttribValueContents = str | PlaceholderData

@dataclass
class TagData:
    name: str
    self_closing: bool
    attributes: Dict[str, List[AttribValueContents]]
    placeholders: List[PlaceholderData]

    def inner_str(self) -> str:
        s = ""
        for name, value in self.attributes.items():
            val = ""
            for v in value:
                if isinstance(v, str):
                    val += v.replace('"', "&quot;")
                else:
                    assert isinstance(v, PlaceholderData)
                    val += str(v)
            if len(value) == 0:
                s += " " + name
            else:
                s += " " + name + '="' + val + '"'
        for placeholder in self.placeholders:
            s += " " + str(placeholder)
        if self.self_closing:
            s += " /"
        return s

type TokenData = DoctypeData | TagData | PlaceholderData | str | None


@dataclass
class Token:
    kind: Tok
    data: TokenData

    def __str__(self) -> str:
        match self.kind:
            case Tok.DOCTYPE:
                assert isinstance(self.data, DoctypeData)
                return str(self.data)
            case Tok.START_TAG:
                assert isinstance(self.data, TagData)
                s = "<" + self.data.name + self.data.inner_str()
                return s + ">"
            case Tok.START_COMPONENT:
                assert isinstance(self.data, TagData)
                s = "<{" + self.data.name + "}" + self.data.inner_str()
                return s + ">"
            case Tok.END_TAG:
                assert self.data is not None and isinstance(self.data, TagData)
                return f"</{self.data.name}>"
            case Tok.END_COMPONENT:
                assert self.data is not None and isinstance(self.data, TagData)
                return f"</{{{self.data.name}}}>"
            case Tok.COMMENT:
                assert isinstance(self.data, str)
                return f"<!--{self.data}-->"
            case Tok.CHARACTER:
                assert isinstance(self.data, str)
                return self.data
            case Tok.PLACEHOLDER:
                assert isinstance(self.data, PlaceholderData)
                return str(self.data)
            case Tok.EOF:
                return ""
