from typing import Callable, Tuple


def is_leading_surrogate(c: int):
    return c >= 0xD800 and c <= 0xDBFF


def is_trailing_surrogate(c: int):
    return c >= 0xDC00 and c <= 0xDFFF


def is_surrogate(c: int):
    return is_leading_surrogate(c) or is_trailing_surrogate(c)


def is_nonchar(c: int):
    if c >= 0xFDD0 and c <= 0xFDEF:
        return True
    for i in range(0x11):
        if c >= i * 0x10000 + 0xFFFE and c <= i * 0x10000 + 0xFFFF:
            return True
    return False


def is_control(c: int):
    return c >= 0x7F and c <= 0x9F or c >= 0 and c <= 0x1F


def is_ascii_whitespace(c: str):
    c = c[0]
    return c in ["\t", "\n", "\f", "\r", " "]


def strip_newlines(s: str) -> str:
    return s.replace("\n", "").replace("\r", "")


def normalize_newlines(s: str) -> str:
    return s.replace("\r\n", "\n").replace("\r", "\n")


def strip_whitespace(s: str) -> str:
    if len(s) == 0:
        return s
    while len(s) > 0 and is_ascii_whitespace(s[0]):
        s = s[1:]
    if len(s) == 0:
        return s
    while len(s) > 0 and is_ascii_whitespace(s[-1]):
        s = s[:-1]
    return s


def strip_collapse_whitespace(s: str) -> str:
    out_s = ""
    whitespace = False
    while len(s) > 0:
        c = s[0]
        s = s[1:]
        if is_ascii_whitespace(c):
            if not whitespace:
                out_s += " "
            whitespace = True
            continue
        whitespace = False
        out_s += c
    return strip_whitespace(out_s)


def collect_while(s: str, cond: Callable[[str], bool]) -> Tuple[str, str]:
    result = ""
    while cond(s[0]):
        result += s[0]
        s = s[1:]
    return result, s

CC_MAP = {
    0x80: 0x20AC,
    0x82: 0x201A,
    0x83: 0x0192,
    0x84: 0x201E,
    0x85: 0x2026,
    0x86: 0x2020,
    0x87: 0x2021,
    0x88: 0x02C6,
    0x89: 0x2030,
    0x8A: 0x0160,
    0x8B: 0x2039,
    0x8C: 0x0152,
    0x8E: 0x017D,
    0x91: 0x2018,
    0x92: 0x2019,
    0x93: 0x201C,
    0x94: 0x201D,
    0x95: 0x2022,
    0x96: 0x2013,
    0x97: 0x2014,
    0x98: 0x02DC,
    0x99: 0x2122,
    0x9A: 0x0161,
    0x9B: 0x203A,
    0x9C: 0x0153,
    0x9E: 0x017E,
    0x9F: 0x0178,
}
