#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "binmap.h"


void usage() {
    fprintf(stderr, "usage: bitmap1 [bitmap2 [...]] ");
    exit(-1);
}


void process(const char * filename, FILE * fin) {
    binmap_t binmap;

    size_t size = 0;
    size_t count = 0;

    for( ;; ) {
        char buf[4096];

        const size_t bytes = fread(buf, 1, sizeof(buf), fin);
        if( bytes == 0 )
            break;

        for(size_t i = 0; i < bytes; ++i) {
            const size_t v = 16 * (size + i);
            const char b = buf[i];

            if( b & 0x01 ) {
                binmap.set(bin_t(v +  0));
                ++count;
            }
            if( b & 0x02 ) {
                binmap.set(bin_t(v +  2));
                ++count;
            }
            if( b & 0x04 ) {
                binmap.set(bin_t(v +  4));
                ++count;
            }
            if( b & 0x08 ) {
                binmap.set(bin_t(v +  6));
                ++count;
            }
            if( b & 0x10 ) {
                binmap.set(bin_t(v +  8));
                ++count;
            }
            if( b & 0x20 ) {
                binmap.set(bin_t(v + 10));
                ++count;
            }
            if( b & 0x40 ) {
                binmap.set(bin_t(v + 12));
                ++count;
            }
            if( b & 0x80 ) {
                binmap.set(bin_t(v + 14));
                ++count;
            }
        }

        size += bytes;
    }

    if( ferror(fin) ) {
        fprintf(stderr, "%s: %d: %s\n", filename, errno, strerror(errno));

    } else if( size == 0 ) {
        printf("Bitmap ``%s'' statistics\n", filename);
        printf("  bitmap is empty\n\n");

    } else {
        printf("Bitmap ``%s'' statistics\n", filename);
        printf("  bitmap filling: %.2f%%\n", 100.0 * count / 8 / size);
        printf("  bitmap size: %u bytes\n", size);
        printf("  binmap size: %u bytes\n", binmap.total_size());
        printf("  binmap size efficiency: %.2f%%\n", 100.0 * binmap.total_size() / size);
        printf("  binmap packed size: %u bytes\n", sizeof(binmap) + sizeof(cell_t) * binmap.cells_number());
        printf("  binmap packed size efficiency: %.2f%%\n", 100.0 * (sizeof(binmap) + sizeof(cell_t) * binmap.cells_number()) / size);

        printf("\n");
    }
}


int main(int argc, char ** argv) {
    if( argc == 1 )
        usage();

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
