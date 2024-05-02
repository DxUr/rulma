#include "hash.h"
#include <stdint.h>


uint32_t hashFNV1AStr(const char *p_str) {
    int hash = 0x811C9DC5;
    for (; *p_str != '\0'; p_str++) {
        hash ^= (int)*p_str;
        hash *= 0x01000193;
    }
    return hash;
}

uint64_t hashBase53(uint32_t p_val) {
    uint64_t _id = 0;
    uint8_t *id = (uint8_t*)&_id;
    const char base53[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_";
    id[5] = base53[p_val / 418195493];
    p_val %= 418195493;
    id[4] = base53[p_val / 7890481];
    p_val %= 7890481;
    id[3] = base53[p_val / 148877];
    p_val %= 148877;
    id[2] = base53[p_val / 2809];
    p_val %= 2809;
    id[1] = base53[p_val / 53];
    p_val %= 53;
    id[0] = base53[p_val];
    return *(uint64_t*)id;
}

