#ifndef LITERAL_H
#define LITERAL_H


typedef enum {
    LT_INT,
    LT_FLOAT,
    LT_STRING
} LT_TYPE ;

typedef struct literal Literal;

Literal *literalCreate(const LT_TYPE p_type, const void* p_data);
LT_TYPE literalGetType(const Literal *p_lt);
void *literalGetVal(const Literal* p_lt);
void literalFree(Literal* p_lt);

#endif // LITERAL_H
