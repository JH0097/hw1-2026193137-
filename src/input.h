#ifndef INPUT_H
#define INPUT_H
#include "sort.h"
extern const char *PATTERNS[5];
void makeInput(Record *a, size_t n, int pattern, uint32_t seed);
int validResult(const Record *a, const Record *original, size_t n);
int isStable(const Record *a, size_t n);
uint64_t resultHash(const Record *a, size_t n);
#endif
