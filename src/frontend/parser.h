#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"

typedef struct Parser Parser;


Parser* parserInit(Tokenizer *p_tokenizer);
int parserParse(Parser *p_parser);
void parserTerminate(Parser *p_parser);


#endif // PARSER_H
