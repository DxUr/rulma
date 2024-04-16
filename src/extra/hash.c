#include "hash.h"



int hashFNV1AStr(const char *p_str) {
    int hash = 0x811C9DC5;
    for (; *p_str != '\0'; p_str++) {
        hash ^= (int)*p_str;
        hash *= 0x01000193;
    }
    return hash;
}

