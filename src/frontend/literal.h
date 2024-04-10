#ifndef LITERAL_H
#define LITERAL_H


typedef enum {
    LT_INT,
    LT_FLOAT,
    LT_STRING
} LiteralType;

typedef struct Literal Literal;

Literal *literalCreate(const LiteralType p_type, const void* p_data);
LiteralType literalGetType(const Literal *p_lt);
void *literalGetVal(const Literal* p_lt);
void literalFree(Literal* p_lt);

#endif // LITERAL_H
