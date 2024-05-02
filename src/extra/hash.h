#ifndef HASH_H
#define HASH_H
#include <stdint.h>


uint32_t hashFNV1AStr(const char *p_str);
uint64_t hashBase53(uint32_t p_val);
#endif // HASH_H
