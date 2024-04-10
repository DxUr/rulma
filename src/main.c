#include <laMa.h>

#include "frontend/tokenizer.h"
#include "frontend/parser.h"

#include <stdio.h>

char get_char(void *p_ctx) {
    return (char)getc((FILE*)p_ctx);
}

int main(int argc, char *argv[]) {

    FILE *file = fopen(argv[1], "rb");

    // Init the tokenizer
    Tokenizer *tk = tokenizerInit(get_char, (void*)file, argv[1]);
    
    Parser *pr = parserInit(tk);

    parserParse(pr);

    parserTerminate(pr);

    /*
    while (tokenizerTokenGetType(tokenizerAdvance(tk)) != TK_EOF) {
        Token* t = tokenizerGetCurrent(tk);
        printf("TOKEN: %s\n", tokenizerTokenGetTypeName(t));
        switch (tokenizerTokenGetType(t)) {
            case TK_ERROR:
                printf("err: %s\n", tokenizerTokenGetErrorString(t));
                break;
            case TK_IDENTIFIER:
            case TK_LITERAL:;
                Literal *lt = tokenizerTokenGetLiteral(t);
                switch (literalGetType(lt)) {
                    case LT_INT:
                        printf("int: %d\n", *(int*)literalGetVal(lt));
                        break;
                    case LT_FLOAT:
                        printf("float: %f\n", *(float*)literalGetVal(lt));
                        break;
                    case LT_STRING:
                        printf("string: %s\n", (char*)literalGetVal(lt));
                }
                break;
            default:
                break;
        }
    }
    
    printf("TOKEN: %s\n", tokenizerTokenGetTypeName(tokenizerGetCurrent(tk)));
    */
    
    tokenizerTerminate(tk);

    return 0;
}
