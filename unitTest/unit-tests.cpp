#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>

#include <gtest/gtest.h>

#include "bin.h"
#include "binmap.h"
#include "cRandom/crandom.h"


struct cRandom * crandom = NULL;


TEST(bin_test, layer) {
    EXPECT_EQ( 0, bin_t( 0).layer() );
    EXPECT_EQ( 1, bin_t( 1).layer() );
    EXPECT_EQ( 0, bin_t( 2).layer() );
    EXPECT_EQ( 2, bin_t( 3).layer() );
    EXPECT_EQ( 0, bin_t( 4).layer() );
    EXPECT_EQ( 1, bin_t( 5).layer() );
    EXPECT_EQ( 0, bin_t( 6).layer() );
    EXPECT_EQ( 3, bin_t( 7).layer() );
    EXPECT_EQ( 0, bin_t( 8).layer() );
    EXPECT_EQ( 1, bin_t( 9).layer() );
    EXPECT_EQ( 0, bin_t(10).layer() );
    EXPECT_EQ( 2, bin_t(11).layer() );
    EXPECT_EQ( 0, bin_t(12).layer() );
    EXPECT_EQ( 1, bin_t(13).layer() );
    EXPECT_EQ( 0, bin_t(14).layer() );
    EXPECT_EQ( 4, bin_t(15).layer() );
    EXPECT_EQ( 8 * sizeof(bin_t::uint_t) - 1, bin_t::ALL.layer() );
    EXPECT_EQ( -1, bin_t::NONE.layer() );

    int l = 5;
    bin_t::uint_t v = 31;
    do {
        EXPECT_EQ( l, bin_t(v).layer() );
        l += 1;
        v += v + 1;
    } while( v + 1 != 0 );
}


TEST(bin_test, layer_bits) {
    EXPECT_EQ(  1, bin_t( 0).layer_bits() );
    EXPECT_EQ(  3, bin_t( 1).layer_bits() );
    EXPECT_EQ(  1, bin_t( 2).layer_bits() );
    EXPECT_EQ(  7, bin_t( 3).layer_bits() );
    EXPECT_EQ(  1, bin_t( 4).layer_bits() );
    EXPECT_EQ(  3, bin_t( 5).layer_bits() );
    EXPECT_EQ(  1, bin_t( 6).layer_bits() );
    EXPECT_EQ( 15, bin_t( 7).layer_bits() );
    EXPECT_EQ(  1, bin_t( 8).layer_bits() );
    EXPECT_EQ(  3, bin_t( 9).layer_bits() );
    EXPECT_EQ(  1, bin_t(10).layer_bits() );
    EXPECT_EQ(  7, bin_t(11).layer_bits() );
    EXPECT_EQ(  1, bin_t(12).layer_bits() );
    EXPECT_EQ(  3, bin_t(13).layer_bits() );
    EXPECT_EQ(  1, bin_t(14).layer_bits() );
    EXPECT_EQ( 31, bin_t(15).layer_bits() );
    EXPECT_EQ( -1, bin_t::ALL.layer_bits() );
    //EXPECT_EQ( 0, bin_t::NONE.layer_bits() );

    bin_t::uint_t lbs = 63;
    bin_t::uint_t v = 31;
    do {
        EXPECT_EQ( lbs, bin_t(v).layer_bits() );
        lbs += lbs + 1;
        v += v + 1;
    } while( v + 1 != 0 );
}


TEST(bin_test, left) {
    EXPECT_TRUE( bin_t( 0) == bin_t( 0).left() );
    EXPECT_TRUE( bin_t( 0) == bin_t( 1).left() );
    EXPECT_TRUE( bin_t( 2) == bin_t( 2).left() );
    EXPECT_TRUE( bin_t( 1) == bin_t( 3).left() );
    EXPECT_TRUE( bin_t( 4) == bin_t( 4).left() );
    EXPECT_TRUE( bin_t( 4) == bin_t( 5).left() );
    EXPECT_TRUE( bin_t( 6) == bin_t( 6).left() );
    EXPECT_TRUE( bin_t( 3) == bin_t( 7).left() );
    EXPECT_TRUE( bin_t( 8) == bin_t( 8).left() );
    EXPECT_TRUE( bin_t( 8) == bin_t( 9).left() );
    EXPECT_TRUE( bin_t(10) == bin_t(10).left() );
    EXPECT_TRUE( bin_t( 9) == bin_t(11).left() );
    EXPECT_TRUE( bin_t(12) == bin_t(12).left() );
    EXPECT_TRUE( bin_t(12) == bin_t(13).left() );
    EXPECT_TRUE( bin_t(14) == bin_t(14).left() );
    EXPECT_TRUE( bin_t( 7) == bin_t(15).left() );
    EXPECT_FALSE( bin_t::ALL == bin_t::ALL.left() );
    EXPECT_TRUE( bin_t::NONE == bin_t::NONE.left() );
}


TEST(bin_test, right) {
    EXPECT_TRUE( bin_t( 0) == bin_t( 0).right() );
    EXPECT_TRUE( bin_t( 2) == bin_t( 1).right() );
    EXPECT_TRUE( bin_t( 2) == bin_t( 2).right() );
    EXPECT_TRUE( bin_t( 5) == bin_t( 3).right() );
    EXPECT_TRUE( bin_t( 4) == bin_t( 4).right() );
    EXPECT_TRUE( bin_t( 6) == bin_t( 5).right() );
    EXPECT_TRUE( bin_t( 6) == bin_t( 6).right() );
    EXPECT_TRUE( bin_t(11) == bin_t( 7).right() );
    EXPECT_TRUE( bin_t( 8) == bin_t( 8).right() );
    EXPECT_TRUE( bin_t(10) == bin_t( 9).right() );
    EXPECT_TRUE( bin_t(10) == bin_t(10).right() );
    EXPECT_TRUE( bin_t(13) == bin_t(11).right() );
    EXPECT_TRUE( bin_t(12) == bin_t(12).right() );
    EXPECT_TRUE( bin_t(14) == bin_t(13).right() );
    EXPECT_TRUE( bin_t(14) == bin_t(14).right() );
    EXPECT_TRUE( bin_t(23) == bin_t(15).right() );
    EXPECT_FALSE( bin_t::ALL == bin_t::ALL.right() );
    EXPECT_TRUE( bin_t::NONE == bin_t::NONE.right() );
}


TEST(bin_test, parent) {
    EXPECT_TRUE( bin_t( 1) == bin_t( 0).parent() );
    EXPECT_TRUE( bin_t( 3) == bin_t( 1).parent() );
    EXPECT_TRUE( bin_t( 1) == bin_t( 2).parent() );
    EXPECT_TRUE( bin_t( 7) == bin_t( 3).parent() );
    EXPECT_TRUE( bin_t( 5) == bin_t( 4).parent() );
    EXPECT_TRUE( bin_t( 3) == bin_t( 5).parent() );
    EXPECT_TRUE( bin_t( 5) == bin_t( 6).parent() );
    EXPECT_TRUE( bin_t(15) == bin_t( 7).parent() );
    EXPECT_TRUE( bin_t( 9) == bin_t( 8).parent() );
    EXPECT_TRUE( bin_t(11) == bin_t( 9).parent() );
    EXPECT_TRUE( bin_t( 9) == bin_t(10).parent() );
    EXPECT_TRUE( bin_t( 7) == bin_t(11).parent() );
    EXPECT_TRUE( bin_t(13) == bin_t(12).parent() );
    EXPECT_TRUE( bin_t(11) == bin_t(13).parent() );
    EXPECT_TRUE( bin_t(13) == bin_t(14).parent() );
    EXPECT_TRUE( bin_t(31) == bin_t(15).parent() );
    EXPECT_TRUE( bin_t::NONE == bin_t::ALL.parent() );
    EXPECT_TRUE( bin_t::NONE == bin_t::NONE.parent() );
}


TEST(bin_test, left_right_parent) {
    const size_t N = 1024;

    for(size_t n = 0; n < N; ++n) {
        const bin_t bin( static_cast<bin_t::uint_t>(uniform(crandom, 0, bin_t::ALL.toUInt() - 1)) );

        EXPECT_TRUE( bin == bin.parent().left() || bin == bin.parent().right() );
    }
}


TEST(bin_test, contains) {
    EXPECT_TRUE( bin_t( 0).contains(bin_t( 0)) );
    EXPECT_TRUE( bin_t( 1).contains(bin_t( 0)) );
    EXPECT_TRUE( bin_t( 1).contains(bin_t( 1)) );
    EXPECT_TRUE( bin_t( 1).contains(bin_t( 2)) );
    EXPECT_TRUE( bin_t( 2).contains(bin_t( 2)) );
    EXPECT_TRUE( bin_t( 3).contains(bin_t( 0)) );
    EXPECT_TRUE( bin_t( 3).contains(bin_t( 1)) );
    EXPECT_TRUE( bin_t( 3).contains(bin_t( 2)) );
    EXPECT_TRUE( bin_t( 3).contains(bin_t( 3)) );
    EXPECT_TRUE( bin_t( 3).contains(bin_t( 4)) );
    EXPECT_TRUE( bin_t( 3).contains(bin_t( 5)) );
    EXPECT_TRUE( bin_t( 3).contains(bin_t( 6)) );
    EXPECT_TRUE( bin_t( 4).contains(bin_t( 4)) );
    EXPECT_TRUE( bin_t( 5).contains(bin_t( 4)) );
    EXPECT_TRUE( bin_t( 5).contains(bin_t( 5)) );
    EXPECT_TRUE( bin_t( 5).contains(bin_t( 6)) );
    EXPECT_TRUE( bin_t( 6).contains(bin_t( 6)) );
    EXPECT_TRUE( bin_t( 7).contains(bin_t( 0)) );
    EXPECT_TRUE( bin_t( 7).contains(bin_t( 1)) );
    EXPECT_TRUE( bin_t( 7).contains(bin_t( 2)) );
    EXPECT_TRUE( bin_t( 7).contains(bin_t( 3)) );
    EXPECT_TRUE( bin_t( 7).contains(bin_t( 4)) );
    EXPECT_TRUE( bin_t( 7).contains(bin_t( 5)) );
    EXPECT_TRUE( bin_t( 7).contains(bin_t( 6)) );
    EXPECT_TRUE( bin_t( 7).contains(bin_t( 7)) );
    EXPECT_TRUE( bin_t( 7).contains(bin_t( 8)) );
    EXPECT_TRUE( bin_t( 7).contains(bin_t( 9)) );
    EXPECT_TRUE( bin_t( 7).contains(bin_t(10)) );
    EXPECT_TRUE( bin_t( 7).contains(bin_t(11)) );
    EXPECT_TRUE( bin_t( 7).contains(bin_t(12)) );
    EXPECT_TRUE( bin_t( 7).contains(bin_t(13)) );
    EXPECT_TRUE( bin_t( 7).contains(bin_t(14)) );
    EXPECT_TRUE( bin_t( 8).contains(bin_t( 8)) );
    EXPECT_TRUE( bin_t( 9).contains(bin_t( 8)) );
    EXPECT_TRUE( bin_t( 9).contains(bin_t( 9)) );
    EXPECT_TRUE( bin_t( 9).contains(bin_t(10)) );
    EXPECT_TRUE( bin_t(10).contains(bin_t(10)) );
    EXPECT_TRUE( bin_t(11).contains(bin_t( 8)) );
    EXPECT_TRUE( bin_t(11).contains(bin_t( 9)) );
    EXPECT_TRUE( bin_t(11).contains(bin_t(10)) );
    EXPECT_TRUE( bin_t(11).contains(bin_t(11)) );
    EXPECT_TRUE( bin_t(11).contains(bin_t(12)) );
    EXPECT_TRUE( bin_t(12).contains(bin_t(12)) );
    EXPECT_TRUE( bin_t(13).contains(bin_t(12)) );
    EXPECT_TRUE( bin_t(13).contains(bin_t(13)) );
    EXPECT_TRUE( bin_t(13).contains(bin_t(14)) );
    EXPECT_TRUE( bin_t(14).contains(bin_t(14)) );


    EXPECT_TRUE( bin_t::ALL.contains(bin_t( 1)) );
    EXPECT_TRUE( bin_t::ALL.contains(bin_t( 2)) );
    EXPECT_TRUE( bin_t::ALL.contains(bin_t( 3)) );
    EXPECT_TRUE( bin_t::ALL.contains(bin_t( 4)) );
    EXPECT_TRUE( bin_t::ALL.contains(bin_t( 5)) );
    EXPECT_TRUE( bin_t::ALL.contains(bin_t( 6)) );
    EXPECT_TRUE( bin_t::ALL.contains(bin_t( 7)) );
    EXPECT_TRUE( bin_t::ALL.contains(bin_t( 8)) );
    EXPECT_TRUE( bin_t::ALL.contains(bin_t( 9)) );
    EXPECT_TRUE( bin_t::ALL.contains(bin_t(10)) );
    EXPECT_TRUE( bin_t::ALL.contains(bin_t(11)) );
    EXPECT_TRUE( bin_t::ALL.contains(bin_t(12)) );
    EXPECT_TRUE( bin_t::ALL.contains(bin_t(13)) );
    EXPECT_TRUE( bin_t::ALL.contains(bin_t(14)) );
    EXPECT_TRUE( bin_t::ALL.contains(bin_t::ALL) );

    EXPECT_FALSE( bin_t::NONE.contains(bin_t( 1)) );
    EXPECT_FALSE( bin_t::NONE.contains(bin_t( 2)) );
    EXPECT_FALSE( bin_t::NONE.contains(bin_t( 3)) );
    EXPECT_FALSE( bin_t::NONE.contains(bin_t( 4)) );
    EXPECT_FALSE( bin_t::NONE.contains(bin_t( 5)) );
    EXPECT_FALSE( bin_t::NONE.contains(bin_t( 6)) );
    EXPECT_FALSE( bin_t::NONE.contains(bin_t( 7)) );
    EXPECT_FALSE( bin_t::NONE.contains(bin_t( 8)) );
    EXPECT_FALSE( bin_t::NONE.contains(bin_t( 9)) );
    EXPECT_FALSE( bin_t::NONE.contains(bin_t(10)) );
    EXPECT_FALSE( bin_t::NONE.contains(bin_t(11)) );
    EXPECT_FALSE( bin_t::NONE.contains(bin_t(12)) );
    EXPECT_FALSE( bin_t::NONE.contains(bin_t(13)) );
    EXPECT_FALSE( bin_t::NONE.contains(bin_t(14)) );
    EXPECT_FALSE( bin_t::NONE.contains(bin_t::ALL) );
    EXPECT_FALSE( bin_t::NONE.contains(bin_t::NONE) );

    const size_t N = 1024;

    for(size_t n = 0; n < N; ++n) {
        const bin_t a( static_cast<bin_t::uint_t>(uniform(crandom, 0, bin_t::ALL.toUInt())) );
        const bin_t b( static_cast<bin_t::uint_t>(uniform(crandom, 0, bin_t::ALL.toUInt())) );
        const bin_t c( static_cast<bin_t::uint_t>(uniform(crandom, 0, bin_t::ALL.toUInt())) );

        EXPECT_TRUE( a.contains(a) && b.contains(b) && c.contains(c) );
        EXPECT_TRUE( !(a.contains(b) && b.contains(c)) || a.contains(c) );
    }
}


TEST(binmap_test, set_get) {
    const size_t N = 33 * 65536;

    uint32_t * const bitmap = new uint32_t[ N / 32];
    memset(bitmap, 0, N / 8);

    binmap_t binmap;

    /* Making random filling */
    for(size_t i = 0; i < 5 * N; ++i) {
        const int n = equilikely(crandom, 0, N - 1);

        binmap.set(bin_t(2 * n));
        bitmap[n / 32] |= 1 << (n % 32);
    }

    /* Checking results */
    for(size_t n = 0; n < N; ++n) {
        const bool f1 = binmap.get(bin_t(static_cast<bin_t::uint_t>(2 * n)));
        const bool f2 = (0 != (bitmap[n / 32] & (1 << (n % 32))));

        EXPECT_EQ( f2, f1 );
    }

    delete [] bitmap;
}


TEST(binmap_test, set_reset_get) {
    const size_t N = 33 * 65536;

    uint32_t * const bitmap = new uint32_t[ N / 32];
    memset(bitmap, 0, N / 8);

    binmap_t binmap;

    /* Making random filling */
    for(size_t i = 0; i < 5 * N; ++i) {
        const int n = equilikely(crandom, 0, N - 1);

        if( bernoulli(crandom, 0.5) ) {
            binmap.set(bin_t(2 * n));
            bitmap[n / 32] |= (1 << (n % 32));
        } else {
            binmap.reset(bin_t(2 * n));
            bitmap[n / 32] &= ~(1 << (n % 32));
        }
    }

    /* Checking results */
    for(size_t n = 0; n < N; ++n) {
        const bool f1 = binmap.get(bin_t(static_cast<bin_t::uint_t>(2 * n)));
        const bool f2 = (0 != (bitmap[n / 32] & (1 << (n % 32))));

        EXPECT_EQ( f2, f1 );
    }

    delete [] bitmap;
}


TEST(binmap_test, find_empty) {
    const size_t N = 33 * 65536;
    binmap_t binmap;

    /* Making random filling */
    for(size_t i = 0; i < 3 * N; ++i) {
        const int n = equilikely(crandom, 0, N - 1);

        binmap.set(bin_t(2 * n));
    }

    /* Checking results */
    size_t cells_number = binmap.cells_number();

    while( binmap.cells_number() != 1 ) {
        const bin_t bin = binmap.find_empty();

        EXPECT_FALSE( binmap.get(bin) );

        binmap.set(bin);

        if ( binmap.cells_number() < cells_number ) {
            cells_number = binmap.cells_number();
        } else {
            EXPECT_LE( binmap.cells_number(), cells_number + 64 );
        }
    }
}


int main(int argc, char ** argv) {
    testing::InitGoogleTest(&argc, argv);

    crandom = dSFMTRandomNew();
    assert( crandom );

    return RUN_ALL_TESTS();
}
