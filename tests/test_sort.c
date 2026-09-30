#include "input.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks=0, cases=0;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x); exit(1); } } while(0)
static int cmpInt(const void *a,const void *b) {
    int x=*(const int *)a,y=*(const int *)b; return (x>y)-(x<y);
}
static void checkCase(const Record *input,size_t n) {
    Record a[300]; int expected[300]; ++cases;
    for(size_t i=0;i<n;++i) expected[i]=input[i].key;
    qsort(expected,n,sizeof(int),cmpInt);
    for(int k=0;k<3;++k) {
        memcpy(a,input,n*sizeof(*a)); SortStats s={0}; SORT_ALGORITHMS[k].sort(a,n,&s);
        CHECK(validResult(a,input,n));
        int equal=1; for(size_t i=0;i<n;++i) if(a[i].key!=expected[i]) equal=0;
        CHECK(equal);
        if(SORT_ALGORITHMS[k].stable) CHECK(isStable(a,n));
    }
}
int main(void) {
    Record a[300];
    for(size_t n=0;n<=7;++n) {
        size_t count=1; for(size_t i=0;i<n;++i) count*=3;
        for(size_t code=0;code<count;++code) {
            size_t v=code;
            for(size_t i=0;i<n;++i) { a[i]=(Record){(int)(v%3)-1,(uint32_t)i}; v/=3; }
            checkCase(a,n);
        }
    }
    for(size_t n=0;n<=257;++n) for(int p=0;p<5;++p) {
        makeInput(a,n,p,1234567U+(uint32_t)n); checkCase(a,n);
    }
    Record extremes[]={{INT_MAX,0},{INT_MIN,1},{0,2},{INT_MAX,3},{INT_MIN,4}};
    checkCase(extremes,5);
    for(int k=0;k<3;++k) { SortStats s={0}; SORT_ALGORITHMS[k].sort(NULL,0,&s); CHECK(s.comparisons==0); }
    Record unstable[]={{2,0},{2,1},{1,2}}; SortStats s={0}; heapSort(unstable,3,&s);
    CHECK(!isStable(unstable,3)); CHECK(unstable[0].tag==2 && unstable[1].tag==1 && unstable[2].tag==0);
    Record original[]={{1,0},{2,1}}, corrupt[]={{1,0},{1,0}};
    CHECK(!validResult(corrupt,original,2));
    corrupt[0]=(Record){9,0}; corrupt[1]=(Record){10,1}; CHECK(!validResult(corrupt,original,2));
    corrupt[0]=(Record){1,99}; CHECK(!validResult(corrupt,original,2));
#ifdef COUNT_STATS
    makeInput(a,100,1,1); s=(SortStats){0}; insertionSort(a,100,&s);
    CHECK(s.comparisons==99); CHECK(s.moves==198);
    makeInput(a,100,2,1); s=(SortStats){0}; insertionSort(a,100,&s);
    CHECK(s.comparisons==4950); CHECK(s.moves==5148);
    makeInput(a,17,0,1); s=(SortStats){0}; mergeSort(a,17,&s);
    CHECK(s.buffer_bytes==17*sizeof(Record));
    for(size_t n=0;n<2;++n) { s=(SortStats){0}; mergeSort(a,n,&s); CHECK(s.buffer_bytes==0); }
#endif
    printf("%u input cases, %u checks, 0 failures\n",cases,checks); return 0;
}
