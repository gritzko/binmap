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
 * Bin numbers in the tail111 encoding: meaningless bits in the tail are set to 0111...11, while the head denotes the offset.
 *   bin = 2 ^ (layer + 1) * layer_offset + 2 ^ layer - 1
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

struct bin_t {
public:

    /**
     * Constants
     */
    static const bin_t NONE;
    static const bin_t ALL;


    /**
     * The array must have 64 cells, as it is the max number of peaks possible + 1 (and there are no reasons to assume there will be less in any given case).
     */
    static int peaks(uint64_t length, bin_t * peaks);


    /**
     * Constructor
     */
    bin_t();

    /**
     * Constructor
     */
    bin_t(uint64_t val);

    /**
     * Gets the bin value
     */
    operator uint64_t () const;


    /**
     * Operator equal
     */
    bool operator == (const bin_t & bin) const;


    /**
     * Operator non-equal
     */
    bool operator != (const bin_t & bin) const;


    /**
     * Operator less
     */
    bool operator < (const bin_t & bin) const;


    /**
     * Operator great
     */
    bool operator > (const bin_t & bin) const;


    /**
     * Gets the begining of the bin interval
     */
    uint64_t base_offset() const;


    /**
     * Gets the length of the bin interval
     */
    uint64_t base_length() const;


    /**
     * Gets the bin layer
     */
    uint8_t layer() const;


    /**
     * Gets the bin layer bits
     */
    uint64_t layer_bits() const;


    /**
     * Gets the bin layer offset
     */
    uint64_t layer_offset() const;


    /**
     * Does the bin is none
     */
    bool is_none() const;


    /**
     * Does the bin is all
     */
    bool is_all() const;


    /**
     * Does the bin is base (layer == 0)
     */
    bool is_base() const;


    /**
     * Checks wheither is bin is a left child
     */
    bool is_left () const;


    /**
     * Checks whether is bin is a left child
     */
    bool is_right () const;


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


    /**
     * Performs a permutation
     */
    bin_t twisted(uint64_t mask) const;


    /**
     * Checks for contains
     */
    bool contains(bin_t bin) const;


    /**
     * Checks for ancestoreness
     */
    bool within(bin_t ancs) const;


    /**
     * Gets the left or the right bin, depending whether the destination is
     */
    bin_t towards(bin_t dest) const;


    /**
     * Depth-first in-order binary tree traversal.
     */
    bin_t next_dfsio(uint8_t floor);


private:

    /**
     * Constructor
     */
    bin_t(uint8_t layer, uint64_t offset);

    uint64_t m_v;
};


#endif
