#ifndef BINMAP_H
#define BINMAP_H

#include <cstddef>
#include "bin.h"

#ifndef _MSC_VER
#  include <stdint.h>
#else
typedef unsigned __int32 uint32_t;
#endif


/**
 * Type of bitmap
 */
typedef uint32_t bitmap_t;

/**
 * Type of reference
 */
typedef uint32_t ref_t;


#pragma pack(push, 1)

/**
 * Structure of cell halves
 */
typedef struct {
    union {
        bitmap_t m_bitmap;
        ref_t m_ref;
    };
} half_t;


/**
 * Structure of cells
 */
typedef union {
    struct {
        half_t m_left;
        half_t m_right;
        bool m_is_left_ref : 1;
        bool m_is_right_ref : 1;
        bool m_is_free : 1;
    };
    ref_t m_free_next;
} cell_t;


#pragma pack(pop)

/**
 * Binmap class
 */
class binmap_t {
public:

    /**
     * Constructor
     */
    binmap_t();


    /**
     * Destructor
     */
    ~binmap_t();


    /**
     * Get bins
     */
    bool get(bin_t bin) const;


    /**
     * Set bins
     */
    void set(bin_t bin);


    /**
     * Reset bins
     */
    void reset(bin_t bin);


    /**
     * Get blocks number
     */
    size_t blocks_number() const;


    /**
     * Get cells number
     */
    size_t cells_number() const;


    /**
     * Echo the binmap status to stdout
     */
    void status() const;


private:


    /**
     * Allocates one cell
     */
    ref_t alloc_cell();


    /**
     * Releases the cell
     */
    void free_cell(ref_t cell);


    /**
     * Extend root
     */
    void extend_root();


    /**
     * Unpack the left half of a cell
     */
    ref_t unpack_left_half(ref_t cell);


    /**
     * Unpack the right half of a cell
     */
    ref_t unpack_right_half(ref_t cell);


    /**
     * Pack a trace of cells
     */
    void pack_cells(ref_t * cells);


    /**
     * Pointer to the list of blocks
     */
    cell_t * m_cell;

    /**
     * Number of allocated blocks (16 * cell)
     */
    size_t m_blocks_number;

    /**
     * Number of allocated cells
     */
    size_t m_cells_number;

    /**
     * Front of the free cell list
     */
    ref_t m_free_top;

    /**
     * The root bin
     */
    bin_t m_root_bin;
};

#endif // BINMAP_H
