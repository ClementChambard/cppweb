from tokenizer import Tokenizer, Tok

def main():
    s = ""
    with open("../../html/page/layout.html") as f:
        s = f.read()
    a = Tokenizer(s)
    toks = []
    while True:
        t = a.tok()
        if t.kind == Tok.EOF:
            break
        toks.append(t)
    from pprint import pprint
    pprint(toks)
    [print(tok) for tok in toks]


if __name__ == "__main__":
    main()
