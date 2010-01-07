#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <ctime>

#include "bin.h"
#include "binmap.h"
#include "cRandom/crandom.h"


/* Binmap unit test */
bool test0() {
    const int seed = time(NULL);
    struct cRandom * const crandom = dSFMTRandomNewBySeed(seed);

    const size_t N = 32 * 65536;
    uint32_t * const bitmap = new uint32_t[ N / 32];
    memset(bitmap, 0, N / 8);

    binmap_t binmap;

    /* Making random filling */
    fprintf(stderr, "Generation\n");
    for(size_t i = 0; i < 3 * N; ++i) {
        const int n = equilikely(crandom, 0, N - 1);

        binmap.set(bin_t(2 * n));
        bitmap[n / 32] |= 1 << (n % 32);
    }

    /* Checking results */
    fprintf(stderr, "Checking\n");
    for(size_t n = 0; n < N; ++n) {
        const bool f1 = (FILL_FILLED == binmap.get(bin_t(2 * n)));
        const bool f2 = bitmap[n / 32] & (1 << (n % 32));

        if( f1 != f2 ) {
            fprintf(stderr, "Error: seed = %u\n", seed);

            binmap.status();

            delete [] bitmap;

            return false;
        }
    }

    fprintf(stderr, "Ok: seed = %u\n", seed);

    binmap.status();

    delete [] bitmap;

    return true;
}


/* Binmap test */
void test1() {
    char line[65536];

    binmap_t binmap;

    while( fgets(line, sizeof(line), stdin) ) {
        static char const * const prefix = "  Free blocks: ";
        static size_t const prefix_length = strlen(prefix);

        if( strncmp(line, prefix, prefix_length) != 0 )
            continue;

        char * str = line + prefix_length;

        while( isspace(*str) )
            ++str;

        for( ; *str != '\0'; ) {
            long int begin, end;

            {
                char * tmp =  str;
                begin = strtoul(tmp, &str, 10);
                assert( tmp != str );
                assert( errno == 0 );
            }

            if( *str == '-' ) {
                char * tmp = str + 1;
                end = strtoul(tmp, &str, 10);
                assert( tmp != str );
                assert( errno == 0 );
            } else
                end = begin;

            if( *str == ',' )
                ++str;
            else
                break;

            while( isspace(*str) )
                ++str;


//            printf("%lu:%lu\n", begin, end);
            for(long int n = begin; n <= end; ++n)
                binmap.set(bin_t(2 * n));
        }
    }

    binmap.status();
}

int main() {
//    test0();
    test1();

    return 0;
}
