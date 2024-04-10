#ifndef ERROR_H
#define ERROR_H

#include "tokenizer.h"

void errorExpected(const char *p_expected, const Token *p_found);
void errorExpectedToken(const TokenType p_expected, const Token *p_found);



#endif // ERROR_H
