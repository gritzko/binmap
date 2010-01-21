#ifndef BIN64_H
#define BIN64_H

#include <assert.h>
#ifdef _MSC_VER
#  include "compat/stdint.h"
#else
#  include <stdint.h>
#endif


/**
 * Numbering for (aligned) logarithmical bins.
 *
 * Each number stands for an interval
 *   [layer_offset * 2^layer, (layer_offset + 1) * 2^layer).
 *
 * The following value is called as base_offset:
 *   layer_offset * 2^layer -- is called
 *
 * Bin numbers in the tail111 encoding: meaningless bits in
 * the tail are set to 0111...11, while the head denotes the offset.
 * bin = 2 ^ (layer + 1) * layer_offset + 2 ^ layer - 1
 *
 * Thus, 1101 is the bin at layer 1, offset 3 (i.e. fourth).
 */

/**
 *
 *                  +-----------------00111-----------------+
 *                  |                                       |
 *        +-------00011-------+                   +-------01011-------+
 *        |                   |                   |                   |
 *   +--00001--+         +--00101--+         +--01001--+         +--01101--+
 *   |         |         |         |         |         |         |         |
 * 00000     00010     00100     00110     01000     01010     01100     1110
 *
 *
 *
 *               7
 *           /       \
 *       3              11
 *     /   \           /  \
 *   1       5       9     13
 *  / \     / \     / \    / \
 * 0   2   4   6   8  10  12 14
 *
 * Once we have peak hashes, this struture is more natural than bin-v1
 *
 */

class bin_t {
public:
    /**
     * Basic integer type
     */
    typedef unsigned int uint_t;


    /**
     * Constants
     */
    static const bin_t NONE;
    static const bin_t ALL;


    /**
     * The array must have 64 cells, as it is the max number
     * of peaks possible + 1 (and there are no reasons to
     * assume there will be less in any given case).  */
//    static int peaks(bint_t length, bin_t * peaks);


    /**
     * Constructor
     */
    bin_t();


    /**
     * Constructor
     */
    explicit bin_t(uint_t val);


    /**
     * Gets the bin value
     */
    uint_t toUInt() const;


    /**
     * Operator equal
     */
    bool operator == (const bin_t & bin) const;


    /**
     * Operator non-equal
     */
    bool operator != (const bin_t & bin) const;


    /**
     * Operator less than
     */
    bool operator < (const bin_t & bin) const;


    /**
     * Operator greater than
     */
    bool operator > (const bin_t & bin) const;


    /**
     * Gets the beginning of the bin(ary interval)
     */
    uint_t base_offset() const;


    /**
     * Gets the length of the bin interval
     */
    uint_t base_length() const;


    /**
     * Gets the bin's layer, i.e. log2(width)
     */
    int layer() const;


    /**
     * Gets the bin layer bits
     */
    uint_t layer_bits() const;


    /**
     * Gets the bin layer offset
     */
    uint_t layer_offset() const;


    /**
     * Whether the bin is none
     */
    bool is_none() const;


    /**
     * Whether the bin is all
     */
    bool is_all() const;


    /**
     * Whether the bin is base (layer == 0)
     */
    bool is_base() const;


    /**
     * Checks wheither is bin is a left child
     */
    bool is_left() const;


    /**
     * Checks whether is bin is a left child
     */
    bool is_right() const;


    /**
     * Gets the sibling bin
     */
    bin_t sibling() const;


    /**
     * Gets the parent bin
     */
    bin_t parent() const;


    /**
     * Gets the left child
     */
    bin_t left() const;


    /**
     * Gets the right child
     */
    bin_t right() const;


    /**
     * Gets the leftmost base sub-bin
     */
    bin_t base_left() const;


//    /**
//     * Performs a permutation
//     */
//    bin_t twisted(bint_t mask) const;


    /**
     * Checks for contains
     */
    bool contains(bin_t bin) const;


    /**
     * Gets the left or the right bin, depending whether the destination is
     */
    bin_t towards(bin_t dest) const;


//    /**
//     * Depth-first in-order binary tree traversal.
//     */
//    bin_t next_dfsio(uint8_t floor);


private:

    /**
     * Constructor
     */
    bin_t(int layer, uint_t offset);

    uint_t m_v;
};



/**
 * Constructor
 */
inline bin_t::bin_t() {
}


/**
 * Constructor
 */
inline bin_t::bin_t(uint_t val) : m_v(val) {
}


/**
 * Gets the layer bits
 */
inline bin_t::uint_t bin_t::layer_bits() const {
    return m_v ^ (m_v + 1);
}


/**
 * Gets the bin value
 */
inline bin_t::uint_t bin_t::toUInt() const {
    return m_v;
}


/**
 * Operator equal
 */
inline bool bin_t::operator == (const bin_t & bin) const {
    return m_v == bin.m_v;
}


/**
 * Operator non-equal
 */
inline bool bin_t::operator != (const bin_t & bin) const {
    return m_v != bin.m_v;
}


/**
 * Operator less
 */
inline bool bin_t::operator < (const bin_t & bin) const {
    return m_v < bin.m_v;
}


/**
 * Operator great
 */
inline bool bin_t::operator > (const bin_t & bin) const {
    return m_v > bin.m_v;
}


/**
 * Gets the parent bin
 */
inline bin_t bin_t::parent() const {
    const uint_t lbs = layer_bits();
    const uint_t nlbs = ~(lbs + 1);

    return bin_t((m_v | lbs) & nlbs);
}


/**
 * Gets the left child
 */
inline bin_t bin_t::left() const {
    const uint_t t = (layer_bits() + 1) >> 2;

//    if (t == 0)
//        return NONE;

    return bin_t(m_v ^ t);
}


/**
 * Gets the right child
 */
inline bin_t bin_t::right() const {
    const uint_t t = (layer_bits() + 1) >> 2;

//    if (t == 0)
//        return NONE;

    return bin_t(m_v + t);
}


/**
 * Does the bin is none
 */
inline bool bin_t::is_none() const {
    return *this == NONE;
}


/**
 * Does the bin is all
 */
inline bool bin_t::is_all() const {
    return *this == ALL;
}


/**
 * Checks is bin is base (layer == 0)
 */
inline bool bin_t::is_base() const {
    return !(m_v & 1);
}


/**
 * Checks is bin is a left child
 */
inline bool bin_t::is_left() const {
    return !(m_v & (layer_bits() + 1));
}


/**
 * Checks wheither is bin is a left child
 */
inline bool bin_t::is_right() const {
    return m_v & (layer_bits() + 1);
}


/**
 * Checks for contains
 */
inline bool bin_t::contains(bin_t bin) const {
    if (is_none())
        return false;

    const uint_t my_bits = layer_bits();
    const uint_t bin_bits = bin.layer_bits();

    return (my_bits >= bin_bits) && ((m_v | my_bits) == (bin.m_v | my_bits));
}

#endif
