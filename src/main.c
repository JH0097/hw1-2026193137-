#define _POSIX_C_SOURCE 200809L
#include "input.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifndef COUNT_STATS
static uint64_t nowNs(void) {
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t)!=0) { perror("clock_gettime"); exit(1); }
    return (uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;
}
#endif
int main(void) {
    const size_t sizes[]={1000,2000,4000,8000,16000};
#ifdef COUNT_STATS
    const size_t batch=1;
#else
    const size_t batch=16;
#endif
    puts("pattern,n,trial,seed,algorithm,ms,batch,comparisons,moves,buffer_bytes,record_bytes,valid,stable,hash");
    for (size_t z=0; z<5; ++z) for (int p=0; p<5; ++p) {
        size_t n=sizes[z];
        Record *base=malloc(n*sizeof(*base)), *work=malloc(batch*n*sizeof(*work));
        if (!base || !work) return 1;
        makeInput(base,n,p,20260930U);
        for (int k=0;k<3;++k) {
            memcpy(work,base,n*sizeof(*work)); SortStats s={0}; SORT_ALGORITHMS[k].sort(work,n,&s);
            if (!validResult(work,base,n)) return 2;
        }
        for (int trial=0; trial<7; ++trial) {
            uint32_t seed=20260930U+(uint32_t)n+(uint32_t)(p*97)+(uint32_t)(trial*1009);
            makeInput(base,n,p,seed);
            for (int offset=0; offset<3; ++offset) {
                int k=(trial+offset)%3;
                /* Allocate/copy all batch inputs before starting the timer. */
                for (size_t b=0;b<batch;++b) memcpy(work+b*n,base,n*sizeof(*work));
                SortStats s={0};
#ifndef COUNT_STATS
                uint64_t start=nowNs();
#endif
                for (size_t b=0;b<batch;++b) SORT_ALGORITHMS[k].sort(work+b*n,n,&s);
#ifdef COUNT_STATS
                double ms=0.0; /* Instrumented binary supplies counts, never timing. */
#else
                double ms=(double)(nowNs()-start)/1e6/(double)batch;
#endif
                uint64_t hash=resultHash(work,n);
                for (size_t b=0;b<batch;++b) {
                    if (!validResult(work+b*n,base,n) || resultHash(work+b*n,n)!=hash) return 2;
                    if (SORT_ALGORITHMS[k].stable && !isStable(work+b*n,n)) return 2;
                }
                printf("%s,%zu,%d,%" PRIu32 ",%s,%.9f,%zu,%" PRIu64 ",%" PRIu64 ",%zu,%zu,1,%d,%" PRIu64 "\n",
                    PATTERNS[p],n,trial,seed,SORT_ALGORITHMS[k].name,ms,batch,s.comparisons,s.moves,
                    s.buffer_bytes,sizeof(Record),isStable(work,n),hash);
            }
        }
        fprintf(stderr,"completed n=%zu %s\n",n,PATTERNS[p]);
        free(base); free(work);
    }
    return 0;
}
