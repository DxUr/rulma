
# trying bottom up expresions parsing

LITERAL = 0
PLUS = 1
MINUS = 2
STAR = 3
PRO = 4 # "("
PRC = 5 # ")"

class Token:
    type = None
    value = None
    def __init__(self, p_type, p_val = None) -> None:
        type = p_type
        value = p_val

tokens = [
    Token(LITERAL, 10),
    Token(PLUS),
    Token(LITERAL, 20),
    Token(PLUS),
    Token(LITERAL, 5),
    Token(STAR),
    Token(LITERAL, 2),
    Token(PLUS),
    Token(MINUS),
    Token(LITERAL, 10),
]

def exp(p_tokens: list):
    token: Token = p_tokens.pop(0)
    match token.type:
        case LITERAL:
            pass
    pass


exp(tokens)

