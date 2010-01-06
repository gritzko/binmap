/*
 *  bin64.cpp
 *  swift
 *
 *  Created by Victor Grishchenko on 10/10/09.
 *  Copyright 2009 Delft University of Technology. All rights reserved.
 *
 */
#include "bin.h"

const bin_t bin_t::NONE(0xffffffffffffffffULL);
const bin_t bin_t::ALL(0x7fffffffffffffffULL);


/* A set of bit operations */

/**
 * Gets the layer bits
 */
uint64_t bin_t::layer_bits() const {
    return m_v ^ (m_v + 1);
}


/**
 * Gets the layer value of a bin
 */
uint8_t bin_t::layer() const {
    uint8_t r = 0;

    uint64_t tail = (layer_bits() + 1) >> 1;

    if (tail > 0xffffffffULL) {
        r = 32;
        tail >>= 32;
    }

    // courtesy of Sean Eron Anderson
    // http://graphics.stanford.edu/~seander/bithacks.html
    static const uint8_t DeBRUIJN[32] = { 0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9 };

    return r + DeBRUIJN[ (static_cast<uint32_t>(tail) * 0x077CB531U) >> 27 ];
}


/**
 * Gets the offset value of a bin
 */
uint64_t bin_t::layer_offset() const {
    return m_v >> (layer() + 1);
}


/* Methods */

/**
 * Constructor
 */
bin_t::bin_t() {
}


/**
 * Constructor
 */
bin_t::bin_t(uint64_t val) : m_v(val) {
}


/**
 * Constructor
 */
bin_t::bin_t(uint8_t layer, uint64_t offset) : m_v( (offset << (layer + 1)) | ((1ULL << layer) - 1) ) {
}


/**
 * Gets the bin value
 */
bin_t::operator uint64_t () const {
    return m_v;
}


/**
 * Operator equal
 */
bool bin_t::operator == (const bin_t & bin) const {
    return m_v == bin.m_v;
}


/**
 * Operator non-equal
 */
bool bin_t::operator != (const bin_t & bin) const {
    return m_v != bin.m_v;
}


/**
 * Operator less
 */
bool bin_t::operator < (const bin_t & bin) const {
    return m_v < bin.m_v;
}


/**
 * Operator great
 */
bool bin_t::operator > (const bin_t & bin) const {
    return m_v > bin.m_v;
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
uint64_t bin_t::base_offset() const {
    return (m_v & (m_v + 1)) >> 1;
}


/**
 * Gets the length of the bin interval
 */
uint64_t bin_t::base_length() const {
    return (layer_bits() >> 1) + 1;
}


/**
 * Does the bin is none
 */
bool bin_t::is_none() const {
    return *this == NONE;
}


/**
 * Does the bin is all
 */
bool bin_t::is_all() const {
    return *this == ALL;
}


/**
 * Checks is bin is base (layer == 0)
 */
bool bin_t::is_base() const {
    return !(m_v & 1);
}


/**
 * Checks is bin is a left child
 */
bool bin_t::is_left() const {
    return !(m_v & (layer_bits() + 1));
}


/**
 * Checks wheither is bin is a left child
 */
bool bin_t::is_right() const {
    return m_v & (layer_bits() + 1);
}


/**
 * Gets the parent bin
 */
bin_t bin_t::parent() const {
    const uint64_t lbs = layer_bits();
    const uint64_t nlbs = ~(lbs + 1);

    return bin_t((m_v | lbs) & nlbs);
}


/**
 * Gets the left child
 */
bin_t bin_t::left() const {
    const uint64_t t = (layer_bits() + 1) >> 2;

    if (t == 0)
        return NONE;

    return bin_t(m_v & (~t));
}


/**
 * Gets the right child
 */
bin_t bin_t::right() const {
    const uint64_t t = (layer_bits() + 1) >> 2;

    if (t == 0)
        return NONE;

    return bin_t(m_v + t);
}


/**
 * Performs a permutation
 */
bin_t bin_t::twisted(uint64_t mask) const {
    return bin_t( m_v ^ ((mask << 1) & ~layer_bits() ) );
}


/**
 * Checks for contains
 */
bool bin_t::contains(bin_t bin) const {
    if (is_none())
        return false;

    const uint64_t my_bits = layer_bits();
    const uint64_t bin_bits = bin.layer_bits();

    return (my_bits >= bin_bits) && ((m_v | my_bits) == (bin.m_v | my_bits));
}


/**
 * Checks for ancestoreness
 */
bool bin_t::within(bin_t ancs) const {
    if (ancs.is_none())
        return false;

    const uint64_t my_bits = layer_bits();
    const uint64_t ancs_bits = ancs.layer_bits();

    return (my_bits < ancs_bits) && ((m_v | ancs_bits) == (ancs.m_v | ancs_bits));
}


/**
 * Gets the left or the right bin, depending whether the destination is
 */
bin_t bin_t::towards(bin_t dest) const {
    if (!dest.within(*this))
        return NONE;
    if (dest.within(left()))
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


/**
 * Depth-first in-order binary tree traversal.
 */
bin_t bin_t::next_dfsio (uint8_t floor) {
    if (is_right())
        return parent();

    bin_t ret = sibling();

    for(uint8_t l = ret.layer(); l > floor; --l)
        ret = ret.left();

    return ret;
}


/**
 * The array must have 64 cells, as it is the max number of peaks possible + 1 (and there are no reasons to assume there will be less in any given case).
 */
int bin_t::peaks(uint64_t length, bin_t * peaks) {
    int pp = 0;
    for(uint8_t layer = 0; length > 0; length >>= 1, layer++) {
        if (length & 1)
            peaks[++pp] = bin_t(layer, length - 1);
    }

    for(int i = (pp >> 1) - 1; i >= 0; --i) {
        uint64_t memo = peaks[pp - 1 - i];
        peaks[pp - 1 - i] = peaks[i];
        peaks[i] = memo;
    }

    peaks[pp] = NONE;

    return pp;
}
