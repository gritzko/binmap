#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "binmap.h"


/* Utils */

static inline fill_t get_fill_type(bitmap_t bitmap) {
    if( bitmap == BITMAP_EMPTY )
        return FILL_EMPTY;
    if( bitmap == BITMAP_FILLED )
        return FILL_FILLED;
    return FILL_MIXED;
}


static inline fill_t get_fill_type(bitmap_t left_bitmap, bitmap_t right_bitmap) {
    if( left_bitmap == right_bitmap )
        return get_fill_type(left_bitmap);
    return FILL_MIXED;
}

static const size_t ROOT_CELL = 0;


/**
 * Get postion of the left half flag
 */
size_t binmap_t::left_n(ref_t cell) {
    return cell / 16;
}

/**
 * Get mask of the left half flag
 */
size_t binmap_t::left_m(ref_t cell) {
    return 0x00000001 << (cell % 16);
}

/**
 * Get position of the right half flag
 */
uint32_t binmap_t::right_n(ref_t cell) {
    return cell / 16;
}

/**
 * Get position of the right half mask
 */
uint32_t binmap_t::right_m(ref_t cell) {
    return 0x00010000 << (cell % 16);
}

/**
 * Check type of the left half
 */
bool binmap_t::is_ref_left(ref_t cell) const {
     return m_halves_flags[ left_n(cell) ] & left_m(cell);
}

/**
 * Check type of the right half
 */
bool binmap_t::is_ref_right(ref_t cell) const {
    return m_halves_flags[ right_n(cell) ] & right_m(cell);
}

/**
 * Check type of the left half
 */
bool binmap_t::is_bitmap_left(ref_t cell) const {
    return !is_ref_left(cell);
}

/**
 * Check type of the right half
 */
bool binmap_t::is_bitmap_right(ref_t cell) const {
    return !is_ref_right(cell);
}


/**
 * Set type of the left half
 */
void binmap_t::set_ref_left(ref_t cell) const {
    m_halves_flags[ left_n(cell) ] |= left_m(cell);
}

/**
 * Set type of the left half
 */
void binmap_t::set_ref_right(ref_t cell) const {
    m_halves_flags[ right_n(cell) ] |= right_m(cell);
}

/**
 * Set type of the left half
 */
void binmap_t::set_bitmap_left(ref_t cell) const {
    m_halves_flags[ left_n(cell) ] &= ~left_m(cell);
}

/**
 * Set type of the left half
 */
void binmap_t::set_bitmap_right(ref_t cell) const {
    m_halves_flags[ right_n(cell) ] &= ~right_m(cell);
}


/* Methods */

/**
 * Constructor
 */
binmap_t::binmap_t() : m_root_bin(1) {
    m_cell = NULL;
    m_halves_flags = NULL;
    m_blocks_number = 0;
    m_cells_number = 0;
    m_free_top = ROOT_CELL;

    const ref_t root_cell = alloc_cell();

    assert( root_cell == 0 && m_blocks_number > 0 );
}


/**
 * Destructor
 */
binmap_t::~binmap_t() {
    if( m_cell )
        free(m_cell);
    if( m_halves_flags )
        free(m_halves_flags);
}


/**
 * Allocates one cell
 */
ref_t binmap_t::alloc_cell() {
    if( m_free_top == ROOT_CELL ) {
        /* Extend the buffer */
        const size_t old_size = m_blocks_number;
        const size_t new_size = (old_size ? 2 * old_size : 1);

        const size_t size1 = 16 * new_size * sizeof(m_cell[0]);
        const size_t size2 = new_size * sizeof(m_halves_flags[0]);

        /* Check for integer overflow */
        if( size1 == 0 )
            return ROOT_CELL /* INTEGER OVERFLOW */;

        /* Reallocate memory */
        cell_t * const cell = static_cast<cell_t *>(realloc(m_cell, size1));
        uint32_t * const is_ref = static_cast<uint32_t *>(realloc(m_halves_flags, size2));

        /* Checking result of memory realocation */
        if( cell )
            m_cell = cell;
        if( is_ref )
            m_halves_flags = is_ref;
        if( cell == NULL || is_ref == NULL )
            return ROOT_CELL /* MEMORY ERROR */;

        m_blocks_number = new_size;

        /* Insert new cells to the free cell list */
        const size_t stop_idx = 16 * old_size - 1;
        size_t idx = 16 * new_size - 1;

        m_cell[ idx-- ].m_free_next = m_free_top;
        for(; idx != stop_idx; --idx)
            m_cell[ idx ].m_free_next = static_cast<ref_t>(idx + 1);

        m_free_top = static_cast<ref_t>(old_size);
    }

    /* Pop an element from the free cell list */
    const ref_t cell = m_free_top;
    m_free_top = m_cell[ m_free_top ].m_free_next;

    /* Clean it */
    memset(&m_cell[ cell ], 0, sizeof(m_cell[0]));
    set_bitmap_left(cell);
    set_bitmap_right(cell);

    ++m_cells_number;

    return cell;
}


/**
 * Releases the cell
 */
void binmap_t::free_cell(ref_t cell) {
    assert( cell > 0 );

    if( is_ref_left(cell) )
        free_cell(m_cell[cell].m_left.m_ref);
    if( is_ref_right(cell) )
        free_cell(m_cell[cell].m_right.m_ref);

    m_cell[ cell ].m_free_next = m_free_top;
    m_free_top = cell;

    --m_cells_number;
}


/**
 * Extend root
 */
void binmap_t::extend_root() {
    assert( m_root_bin != bin_t::ALL );

    if( m_cell[ROOT_CELL].m_left.m_bitmap == m_cell[ROOT_CELL].m_right.m_bitmap ) {
        /* Setup the root cell */
        m_cell[ROOT_CELL].m_right.m_bitmap = BITMAP_EMPTY;

    } else {
        /* Allocate new cell */
        const ref_t cell = alloc_cell();
        if( cell == ROOT_CELL )
            return /* ALLOC ERROR */;

        /* Move old root to the cell */
        memcpy(&m_cell[cell], &m_cell[ROOT_CELL], sizeof(m_cell[0]));
        if( is_ref_left(ROOT_CELL) )
            set_ref_left(cell);
        if( is_ref_right(ROOT_CELL) )
            set_ref_right(cell);

        /* Setup new root */
        set_ref_left(ROOT_CELL);
        m_cell[ROOT_CELL].m_left.m_ref = cell;
        set_bitmap_right(ROOT_CELL);
        m_cell[ROOT_CELL].m_right.m_bitmap = BITMAP_EMPTY;
    }

    /* Reset bin */
    m_root_bin = m_root_bin.parent();
}


/**
 * Unpack the left half of a cell
 */
ref_t binmap_t::unpack_left_half(ref_t cell) {
    if( is_bitmap_left(cell) ) {
        const ref_t left_cell = alloc_cell();
        if( left_cell == ROOT_CELL )
            return ROOT_CELL /* ALLOC ERROR */;

        m_cell[left_cell].m_left.m_bitmap = m_cell[cell].m_left.m_bitmap;
        m_cell[left_cell].m_right.m_bitmap = m_cell[cell].m_left.m_bitmap;

        set_ref_left(cell);
        m_cell[cell].m_left.m_ref = left_cell;
    }

    return m_cell[cell].m_left.m_ref;
}


/**
 * Unpack the right half of a cell
 */
ref_t binmap_t::unpack_right_half(ref_t cell) {
    if( is_bitmap_right(cell) ) {
        const ref_t right_cell = alloc_cell();
        if( right_cell == ROOT_CELL )
            return ROOT_CELL /* ALLOC ERROR */;

        m_cell[right_cell].m_left.m_bitmap = m_cell[cell].m_right.m_bitmap;
        m_cell[right_cell].m_right.m_bitmap = m_cell[cell].m_right.m_bitmap;

        set_ref_right(cell);
        m_cell[cell].m_right.m_ref = right_cell;
    }

    return m_cell[cell].m_right.m_ref;
}


/**
 * Pack a trace of cells
 */
void binmap_t::pack_cells(ref_t * trace_cell) {
    ref_t cell = *trace_cell--;

    if( cell == ROOT_CELL )
        return;

    if( m_cell[cell].m_left.m_bitmap != m_cell[cell].m_right.m_bitmap )
        return;

    const bitmap_t bitmap = m_cell[cell].m_left.m_bitmap;

    do {
        cell = *trace_cell--;

        if( is_bitmap_left(cell) ) {
            if( m_cell[cell].m_left.m_bitmap != bitmap )
                break;

        } else if( is_bitmap_right(cell) ) {
            if( m_cell[cell].m_right.m_bitmap != bitmap )
                break;

        } else
            break;

    } while( cell != ROOT_CELL );

    const ref_t par_cell = trace_cell[2];

    if( is_ref_left(cell) && m_cell[cell].m_left.m_ref == par_cell ) {
        set_bitmap_left(cell);
        m_cell[cell].m_left.m_bitmap = bitmap;
    } else {
        set_bitmap_right(cell);
        m_cell[cell].m_right.m_bitmap = bitmap;
    }

    free_cell(par_cell);
}


/**
 * Get bins
 *
 * @param bin
 *             the bin
 * @return fill type of the bin
 */
fill_t binmap_t::get(bin_t bin) const {
    if( !m_root_bin.contains(bin) )
        return FILL_EMPTY;

    /* Trace the nearest ancestor bin */
    ref_t cur_cell = ROOT_CELL;
    bin_t cur_bin = m_root_bin;

    for( ;; ) {
        if( bin == cur_bin )
            return get_fill_type(m_cell[cur_cell].m_left.m_bitmap, m_cell[cur_cell].m_right.m_bitmap);

        if( bin < cur_bin ) {
            if( is_bitmap_left(cur_cell) )
                break;

            cur_bin = cur_bin.left();
            cur_cell = m_cell[cur_cell].m_left.m_ref;

        } else {
            if( is_bitmap_right(cur_cell) )
                break;

            cur_bin = cur_bin.right();
            cur_cell = m_cell[cur_cell].m_right.m_ref;
        }
    }

    /* Check current bin bitmap */
    // FIXME: if( bin.is_base() )
    if( bin < cur_bin )
        return get_fill_type(m_cell[cur_cell].m_left.m_bitmap);
    return get_fill_type(m_cell[cur_cell].m_right.m_bitmap);
}


/**
 * Sets bins
 *
 * @param bin
 *             the bin
 */
void binmap_t::set(bin_t bin) {
    if( bin.is_none() )
        return;

    /* Extending binmap if needed */
    while( !m_root_bin.contains(bin) )
        extend_root();

    /* Process the special case */
    if( m_root_bin == bin ) {
        if( is_ref_left(ROOT_CELL) ) {
            free_cell(m_cell[ROOT_CELL].m_left.m_ref);
            set_bitmap_left(ROOT_CELL);
        }

        if( is_ref_right(ROOT_CELL) ) {
            free_cell(m_cell[ROOT_CELL].m_right.m_ref);
            set_bitmap_right(ROOT_CELL);
        }

        m_cell[ROOT_CELL].m_left.m_bitmap = BITMAP_FILLED;
        m_cell[ROOT_CELL].m_right.m_bitmap = BITMAP_FILLED;

        return;
    }

    /* Trace the parent bin */
    const bin_t par_bin = bin.parent();

    ref_t _trace_cell[64];
    ref_t * trace_cell = _trace_cell;
    *trace_cell++ = ROOT_CELL;

    ref_t cur_cell = ROOT_CELL;
    bin_t cur_bin = m_root_bin;

    /* Trace in existed tree */
    while( cur_bin != par_bin ) {
        if( par_bin < cur_bin ) {
            if( is_bitmap_left(cur_cell) )
                break;
            cur_cell = m_cell[cur_cell].m_left.m_ref;
            cur_bin = cur_bin.left();

        } else {
            if( is_bitmap_right(cur_cell) )
                break;

            cur_cell = m_cell[cur_cell].m_right.m_ref;
            cur_bin = cur_bin.right();
        }

        *trace_cell++ = cur_cell;
    }

    /* If the parent bin is found */
    if( cur_bin == par_bin ) {
        if( bin.is_left() ) {  // FIXME: optimization (bin < cur_bin)
            if( is_ref_left(cur_cell) ) {
                free_cell(m_cell[cur_cell].m_left.m_ref);
                set_bitmap_left(cur_cell);
            }
            m_cell[cur_cell].m_left.m_bitmap = BITMAP_FILLED;
        } else {
            if( is_ref_right(cur_cell) ) {
                free_cell(m_cell[cur_cell].m_right.m_ref);
                set_bitmap_right(cur_cell);
            }
            m_cell[cur_cell].m_right.m_bitmap = BITMAP_FILLED;
        }

        pack_cells(trace_cell - 1);

        return;
    }

    /* Are we need to do anything? */
    if( par_bin < cur_bin ) {  // FIXME: optimization (par_bin < cur_bin)
        if( m_cell[cur_cell].m_left.m_bitmap == BITMAP_FILLED )
            return;
    } else {
        if( m_cell[cur_cell].m_right.m_bitmap == BITMAP_FILLED )
            return;
    }

    /* Continue to trace with unpacking */
    while( cur_bin != par_bin ) {
        if( par_bin < cur_bin ) {
            cur_cell = unpack_left_half(cur_cell);
            cur_bin = cur_bin.left();
        } else {
            cur_cell = unpack_right_half(cur_cell);
            cur_bin = cur_bin.right();
        }

        if( cur_cell == ROOT_CELL ) {
            pack_cells(trace_cell - 1);
            return; /* UNPACK HALF ERROR */
        }

        *trace_cell++ = cur_cell;
    }

    /* Complete setting */
    if( bin.is_left() )
        m_cell[cur_cell].m_left.m_bitmap = BITMAP_FILLED;
    else
        m_cell[cur_cell].m_right.m_bitmap = BITMAP_FILLED;
}


/**
 * Get blocks number
 */
size_t binmap_t::blocks_number() const {
    return m_blocks_number;
}


/**
 * Get cells number
 */
size_t binmap_t::cells_number() const {
    return m_cells_number;
}


/**
 * Echo the binmap status to stdout
 */
void binmap_t::status() const {
    printf("bitmap: ");
    for(int i = 0; i < 64; ++i)
        printf("%d", get(bin_t(2 * i)));
    printf("\n");

    printf("cells number: %u\n", cells_number());
    printf("blocks number: %u\n", blocks_number());
    printf("root bin: %llu\n", static_cast<uint64_t>(m_root_bin));

    ref_t _nil_cell[256];
    bin_t _nil_bin[256];
    ref_t * top_cell = _nil_cell;
    bin_t * top_bin = _nil_bin;

    *top_cell++ = 0;
    *top_bin++ = m_root_bin;

    while( top_cell != _nil_cell ) {
        const ref_t cell = *(--top_cell);
        const bin_t bin = *(--top_bin);

        printf("Cell: %u, Bin: %llu, Layer: %u\n", cell, static_cast<uint64_t>(bin), static_cast<unsigned int>(bin.layer()));

        if( is_ref_left(cell) ) {
            const ref_t left_ref = m_cell[cell].m_left.m_ref;
            printf(" left ref: %u, bin: %llu\n", left_ref, static_cast<uint64_t>(bin.left()));
            *top_cell++ = left_ref;
            *top_bin++ = bin.left();
        } else {
            const bitmap_t left_bitmap = m_cell[cell].m_left.m_bitmap;
            printf(" left bitmap: %08x\n", left_bitmap);
        }

        if( is_ref_right(cell) ) {
            const ref_t right_ref = m_cell[cell].m_right.m_ref;
            printf(" right ref: %u, bin: %llu\n", right_ref, static_cast<uint64_t>(bin.right()));
            *top_cell++ = right_ref;
            *top_bin++ = bin.right();
        } else {
            const bitmap_t right_bitmap = m_cell[cell].m_right.m_bitmap;
            printf(" right bitmap: %08x\n", right_bitmap);
        }
    }
}
