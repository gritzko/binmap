#include "bin.h"
#include "binmap.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>


/* Binmap unit test */
bool test0() {
    const size_t N = 65536;

    assert( N <= RAND_MAX );

    uint32_t * bitmap = new uint32_t[ N / 32];
    binmap_t binmap;

    unsigned int seed = time(NULL); // 1262801300;
    srandom( seed );

    /* Making random filling */
    fprintf(stderr, "Generation\n");
    for(size_t i = 0; i < 6 * N; ++i) {
        const size_t n = random() % N;

        binmap.set(bin_t(2 * n));
        bitmap[n / 32] |= 1 << (n % 32);
    }

    /* Checking results */
    fprintf(stderr, "Checking\n");
    for(size_t n = 0; n < N; ++n) {
        const bool f1 = (FILL_FILLED == binmap.get(2 * n));
        const bool f2 = bitmap[n / 32] & (1 << (n % 32));

        if( f1 != f2 ) {
            fprintf(stderr, "Error: seed = %u\n", seed);
            return false;
        }
    }

    fprintf(stderr, "Ok: seed = %u\n", seed);

    binmap.status();

    return 0;
}


int main() {
    test0();
//    binmap_t binmap;
//
//    for(int i = 0; i < 100000; ++i)
//        binmap.set(2 * i);
//
//    binmap.status();

//    for(uint64_t bin = 0; bin < 63; ++bin) {
//        binmap_t binmap;
//        binmap.set(bin_t(bin));
//
//        bitmap_t left_bitmap = BITMAP_EMPTY;
//
//        for(uint64_t i = 0; i < 128; ++i) {
//            if( binmap.get(bin_t(2 * i)) == FILL_FILLED )
//                left_bitmap |= 1 << i;
//        }
//
//
//        printf("\t0x%08x,\n", left_bitmap);
//    }

    return 0;
}
