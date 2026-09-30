#include "input.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
const char *PATTERNS[5] = {"random", "sorted", "reverse", "duplicates", "nearly"};
/* Fixed PRNG implementation makes inputs reproducible across C libraries. */
static uint32_t nextRandom(uint32_t *state) {
    uint32_t x = *state; x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    *state = x; return x;
}
static uint32_t bounded(uint32_t *state, uint32_t bound) {
    /* xorshift32 visits 1..UINT32_MAX; reject the incomplete upper interval. */
    uint32_t limit = UINT32_MAX - UINT32_MAX % bound;
    uint32_t x;
    do { x = nextRandom(state); } while (x > limit);
    return (x-1) % bound;
}
void makeInput(Record *a, size_t n, int pattern, uint32_t seed) {
    if (n > INT_MAX || n > UINT32_MAX || pattern < 0 || pattern > 4) {
        fputs("unsupported input size or pattern\n", stderr); exit(1);
    }
    if (!seed) seed = 1;
    for (size_t i=0; i<n; ++i) {
        a[i].key = pattern == 2 ? (int)(n-1-i) : (int)i;
        if (pattern == 3) a[i].key = (int)bounded(&seed, 16);
    }
    if (pattern == 0) {
        for (size_t i=n; i>1; --i) {
            size_t j=bounded(&seed, (uint32_t)i);
            int t=a[i-1].key; a[i-1].key=a[j].key; a[j].key=t;
        }
    }
    if (pattern == 4 && n>1) {
        for (size_t i=0; i<n/100; ++i) {
            size_t j=bounded(&seed, (uint32_t)(n-1));
            int t=a[j].key; a[j].key=a[j+1].key; a[j+1].key=t;
        }
    }
    for (size_t i=0; i<n; ++i) a[i].tag=(uint32_t)i;
}
int validResult(const Record *a, const Record *original, size_t n) {
    unsigned char *seen=calloc(n ? n : 1, 1);
    if (!seen) exit(1);
    int ok=1;
    for (size_t i=0; i<n; ++i) {
        if ((i && a[i-1].key>a[i].key) || a[i].tag>=n) { ok=0; break; }
        if (seen[a[i].tag] || original[a[i].tag].key!=a[i].key) { ok=0; break; }
        seen[a[i].tag]=1;
    }
    free(seen); return ok;
}
int isStable(const Record *a, size_t n) {
    for (size_t i=1; i<n; ++i)
        if (a[i-1].key==a[i].key && a[i-1].tag>a[i].tag) return 0;
    return 1;
}
uint64_t resultHash(const Record *a, size_t n) {
    uint64_t h=UINT64_C(14695981039346656037);
    for (size_t i=0; i<n; ++i) {
        h ^= (uint32_t)a[i].key; h *= UINT64_C(1099511628211);
        h ^= a[i].tag; h *= UINT64_C(1099511628211);
    }
    return h;
}
