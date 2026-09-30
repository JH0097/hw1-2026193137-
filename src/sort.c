#include "sort.h"
#include <stdio.h>
#include <stdlib.h>
/* Timing builds remove counters at preprocessing time, with no counter guards. */
#ifdef COUNT_STATS
#define ADD(s, field, value) ((s)->field += (value))
#define SET(s, field, value) ((s)->field = (value))
#else
#define ADD(s, field, value) ((void)(s))
#define SET(s, field, value) ((void)(s))
#endif
static int greater(Record a, Record b, SortStats *s) {
    ADD(s, comparisons, 1);
    return a.key > b.key;
}
static void swap(Record *a, Record *b, SortStats *s) {
    Record t = *a; *a = *b; *b = t;
    ADD(s, moves, 3);
}
void insertionSort(Record *a, size_t n, SortStats *s) {
    for (size_t i = 1; i < n; ++i) {
        Record t = a[i]; ADD(s, moves, 1);
        size_t j = i;
        while (j > 0 && greater(a[j-1], t, s)) {
            a[j] = a[j-1]; ADD(s, moves, 1); --j;
        }
        a[j] = t; ADD(s, moves, 1);
    }
}
static void mergeRange(Record *a, Record *tmp, size_t lo, size_t hi, SortStats *s) {
    if (hi-lo < 2) return;
    size_t mid = lo+(hi-lo)/2;
    mergeRange(a, tmp, lo, mid, s);
    mergeRange(a, tmp, mid, hi, s);
    size_t i = lo, j = mid, k = lo;
    while (i < mid && j < hi) {
        /* Equal keys come from the left half first. */
        tmp[k++] = greater(a[i], a[j], s) ? a[j++] : a[i++];
        ADD(s, moves, 1);
    }
    while (i < mid) { tmp[k++] = a[i++]; ADD(s, moves, 1); }
    while (j < hi) { tmp[k++] = a[j++]; ADD(s, moves, 1); }
    for (k = lo; k < hi; ++k) { a[k] = tmp[k]; ADD(s, moves, 1); }
}
void mergeSort(Record *a, size_t n, SortStats *s) {
    if (n < 2) return;
    if (n > SIZE_MAX/sizeof(Record)) { fputs("size overflow\n", stderr); exit(1); }
    Record *tmp = malloc(n*sizeof(*tmp));
    if (!tmp) { fputs("allocation failed\n", stderr); exit(1); }
    SET(s, buffer_bytes, n*sizeof(*tmp));
    mergeRange(a, tmp, 0, n, s);
    free(tmp);
}
static void siftDown(Record *a, size_t root, size_t n, SortStats *s) {
    while (root < n/2) { /* A left child exists; avoids size_t overflow. */
        size_t child = 2*root+1;
        if (child+1 < n && greater(a[child+1], a[child], s)) ++child;
        if (!greater(a[child], a[root], s)) break;
        swap(&a[root], &a[child], s);
        root = child;
    }
}
void heapSort(Record *a, size_t n, SortStats *s) {
    if (n < 2) return;
    for (size_t i = n/2; i > 0; --i) siftDown(a, i-1, n, s);
    for (size_t end = n; end > 1; --end) {
        swap(&a[0], &a[end-1], s);
        siftDown(a, 0, end-1, s);
    }
}
const SortAlgorithm SORT_ALGORITHMS[3] = {
    {"insertion", insertionSort, 1}, {"merge", mergeSort, 1}, {"heap", heapSort, 0}
};
