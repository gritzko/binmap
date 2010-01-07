#include "bin.h"
#include "binmap.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <ctime>


/* Binmap unit test */
bool test0() {
    const size_t N = 32 * 65536;

    assert( N <= RAND_MAX );

    uint32_t * bitmap = new uint32_t[ N / 32];
    binmap_t binmap;

    memset(bitmap, 0, N / 8);

    unsigned int seed = time(NULL);
    srandom( seed );

    /* Making random filling */
    fprintf(stderr, "Generation\n");
    for(size_t i = 0; i < 3 * N; ++i) {
        const size_t n = random() % N;

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
