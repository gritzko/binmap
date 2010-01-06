#include "binmap.h"


int main() {
    binmap_t binmap;

    for(int i = 0; i < 127; ++i)
        binmap.set(bin_t(i));

    binmap.status();

    return 0;
}
