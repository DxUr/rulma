#include "error.h"

#include <stdio.h>
#include <stdbool.h>


void errorExpectedToken(const TokenType p_expected, const Token *p_found) {
    printf("\x1b[1m%s:%d \x1b[91merror:\x1b[1;97m Expected \"\x1b[96m%s\x1b[97m\", ", tokenizerTokenGetSource(p_found), tokenizerTokenGetLine(p_found), tokenizerTokenTypeName(p_expected));
    switch (tokenizerTokenGetType(p_found)) {
        case TK_IDENTIFIER:
            printf("found \x1b[96mIDENTFIRE\x1b[97m: \"%s\"", (char*)literalGetVal(tokenizerTokenGetLiteral(p_found)));
            break;
        default:
            printf("found \"\x1b[96m%s\x1b[97m\"", tokenizerTokenGetTypeName(p_found));
    }
    puts("\x1b[0m");
}

void errorExpected(const char *p_expected, const Token *p_found) {
    printf("\x1b[1m%s:%d \x1b[91merror:\x1b[1;97m Expected \x1b[96m%s\x1b[97m, ", tokenizerTokenGetSource(p_found), tokenizerTokenGetLine(p_found), p_expected);
    switch (tokenizerTokenGetType(p_found)) {
        case TK_IDENTIFIER:
            printf("found \x1b[96mIDENTFIRE\x1b[97m: \"%s\"", (char*)literalGetVal(tokenizerTokenGetLiteral(p_found)));
            break;
        default:
            printf("found \"\x1b[96m%s\x1b[97m\"", tokenizerTokenGetTypeName(p_found));
    }
    puts("\x1b[0m");
}
