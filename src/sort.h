#ifndef SORT_H
#define SORT_H
#include <stddef.h>
#include <stdint.h>
/* tag identifies the original position; only key participates in comparisons. */
typedef struct { int key; uint32_t tag; } Record;
typedef struct { uint64_t comparisons, moves; size_t buffer_bytes; } SortStats;
typedef void (*SortFn)(Record *, size_t, SortStats *);
typedef struct { const char *name; SortFn sort; int stable; } SortAlgorithm;
extern const SortAlgorithm SORT_ALGORITHMS[3];
void insertionSort(Record *a, size_t n, SortStats *stats);
void mergeSort(Record *a, size_t n, SortStats *stats);
void heapSort(Record *a, size_t n, SortStats *stats);
#endif
