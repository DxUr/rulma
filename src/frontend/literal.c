#include "literal.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct Literal {
    LiteralType type;
    void* val;
};


Literal *literalCreate(const LiteralType p_type, const void *p_data) {
    Literal *lt = (Literal*)malloc(sizeof(Literal));
    lt->type = p_type;
    switch (p_type) {
        case LT_INT:
            lt->val = malloc(sizeof(int));
            *(int*)lt->val = *(int*)p_data;
            break;
        case LT_FLOAT:
            lt->val = malloc(sizeof(float));
            *(float*)lt->val = *(float*)p_data;
            break;
        case LT_STRING:
            lt->val = malloc(sizeof(char) * strlen(p_data));
            strcpy(lt->val, p_data);
            break;
    }
    return lt;
}

LiteralType literalGetType(const Literal *p_lt) {
    return p_lt->type;
}

void *literalGetVal(const Literal* p_lt) {
    return p_lt->val;
}

const char *literalStringGetVal(const Literal *p_lt) {
    assert(p_lt && p_lt->type);
    return (char*)p_lt->val;
}

void literalFree(Literal *p_lt) {
    if(!p_lt)
        return;
    free(p_lt->val);
	free(p_lt);
}



