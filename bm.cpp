#include <cstdio>
#include <cstddef>
#include <cstring>

#include "cRandom/crandom.h"
#include "binmap.h"


void process(const char * filename, FILE * fin) {
        binmap_t binmap;

        size_t size;

        for( ;; ) {
                char buf[4096];

                const size_t bytes = fread(buf, 1, sizeof(buf), fin);
                if( bytes == 0 )
                        break;

                for(size_t i = 0; i < bytes; ++i) {
                        const size_t v = 16 * (size + i);
                        const char b = buf[i];

                        if( b & 0x01 )
                                binmap.set(bin_t(v +  0));
                        if( b & 0x02 )
                                binmap.set(bin_t(v +  2));
                        if( b & 0x04 )
                                binmap.set(bin_t(v +  4));
                        if( b & 0x08 )
                                binmap.set(bin_t(v +  6));
                        if( b & 0x10 )
                                binmap.set(bin_t(v +  8));
                        if( b & 0x20 )
                                binmap.set(bin_t(v + 10));
                        if( b & 0x40 )
                                binmap.set(bin_t(v + 12));
                        if( b & 0x80 )
                                binmap.set(bin_t(v + 14));
                }

                size += bytes;
        }

        binmap.status();
}


int main(int argc, char ** argv) {
        struct cRandom * const crandom = dSFMTRandomNew();

        for(int i = 1; i < argc; ++i) {
                FILE * const fin = fopen(argv[i], "rb");

                if( fin == NULL ) {
                        fprintf(stderr, "%s: %d: %s\n", argv[i], errno, strerror(errno));
                        continue;
                }

                process(argv[i], fin);

                fclose(fin);
        }
        
        return 0;
}