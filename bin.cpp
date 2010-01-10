/*
 *  bin64.cpp
 *  swift
 *
 *  Created by Victor Grishchenko on 10/10/09.
 *  Copyright 2009 Delft University of Technology. All rights reserved.
 *
 */
#include "bin.h"

const bin_t bin_t::NONE(static_cast<bin_t::uint_t>(0xffffffffffffffffULL));
const bin_t bin_t::ALL(static_cast<bin_t::uint_t>(0xffffffffffffffffULL) >> 1);


/* A set of bit operations */

/**
 * Gets the layer value of a bin
 */
int bin_t::layer() const {
    int r = 0;

    uint_t tail = (layer_bits() + 1) >> 1;

    if( tail > 0xffffffffU ) {
        r = 32;
        tail >>= 16;    // FIXME: hide warning
        tail >>= 16;
    }

    // courtesy of Sean Eron Anderson
    // http://graphics.stanford.edu/~seander/bithacks.html
    static const int DeBRUIJN[32] = { 0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9 };

    return r + DeBRUIJN[ 0x1f & ((tail * 0x077CB531U) >> 27) ];
}


/**
 * Gets the offset value of a bin
 */
bin_t::uint_t bin_t::layer_offset() const {
    return m_v >> (layer() + 1);
}


/* Methods */

/**
 * Constructor
 */
bin_t::bin_t(int layer, uint_t offset) : m_v( (offset << (layer + 1)) | ((1ULL << layer) - 1) ) {
}


/**
 * Gets the sibling bin
 */
bin_t bin_t::sibling() const {
    return bin_t(m_v ^ (layer_bits() + 1));
}


/**
 * Gets a begining of the bin interval
 */
bin_t::uint_t bin_t::base_offset() const {
    return (m_v & (m_v + 1)) >> 1;
}


/**
 * Gets the length of the bin interval
 */
bin_t::uint_t bin_t::base_length() const {
    return (layer_bits() >> 1) + 1;
}


///**
// * Performs a permutation
// */
//bin_t bin_t::twisted(bint_t mask) const {
//    return bin_t( m_v ^ ((mask << 1) & ~layer_bits() ) );
//}


/**
 * Gets the left or the right bin, depending whether the destination is
 */
bin_t bin_t::towards(bin_t dest) const {
    if (*this == dest || !contains(dest))
        return NONE;
    if (dest < *this)
        return left();
    return right();
}


/**
 * Gets the leftmost base sub-bin
 */
bin_t bin_t::base_left() const {
    if (is_none())
        return NONE;

    return bin_t(m_v & (m_v + 1));
}


///**
// * Depth-first in-order binary tree traversal.
// */
//bin_t bin_t::next_dfsio (int floor) {
//    if (is_right())
//        return parent();
//
//    bin_t ret = sibling();
//
//    for(int l = ret.layer(); l > floor; --l)
//        ret = ret.left();
//
//    return ret;
//}


///**
// * The array must have 64 cells, as it is the max number of peaks possible + 1 (and there are no reasons to assume there will be less in any given case).
// */
//int bin_t::peaks(bint_t length, bin_t * peaks) {
//    int pp = 0;
//    for(int layer = 0; length > 0; length >>= 1, layer++) {
//        if (length & 1)
//            peaks[++pp] = bin_t(layer, length - 1);
//    }
//
//    for(int i = (pp >> 1) - 1; i >= 0; --i) {
//        bint_t memo = peaks[pp - 1 - i];
//        peaks[pp - 1 - i] = peaks[i];
//        peaks[i] = memo;
//    }
//
//    peaks[pp] = NONE;
//
//    return pp;
//}
