#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "binmap.h"

/* Constants */

static const uint64_t BITMAP_LAYER_BITS = 63;

static const bitmap_t BITMAP[] = {
    0x00000001, 0x00000003, 0x00000002, 0x0000000f, 0x00000004, 0x0000000c, 0x00000008, 0x000000ff,
    0x00000010, 0x00000030, 0x00000020, 0x000000f0, 0x00000040, 0x000000c0, 0x00000080, 0x0000ffff,
    0x00000100, 0x00000300, 0x00000200, 0x00000f00, 0x00000400, 0x00000c00, 0x00000800, 0x0000ff00,
    0x00001000, 0x00003000, 0x00002000, 0x0000f000, 0x00004000, 0x0000c000, 0x00008000, 0xffffffff,
    0x00010000, 0x00030000, 0x00020000, 0x000f0000, 0x00040000, 0x000c0000, 0x00080000, 0x00ff0000,
    0x00100000, 0x00300000, 0x00200000, 0x00f00000, 0x00400000, 0x00c00000, 0x00800000, 0xffff0000,
    0x01000000, 0x03000000, 0x02000000, 0x0f000000, 0x04000000, 0x0c000000, 0x08000000, 0xff000000,
    0x10000000, 0x30000000, 0x20000000, 0xf0000000, 0x40000000, 0xc0000000, 0x80000000, /* special */ 0xffffffff /* special */
};


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
binmap_t::binmap_t() : m_root_bin(63) {
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

        m_free_top = static_cast<ref_t>(16 * old_size);
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

    /* Trace the bin */
    ref_t cur_cell = ROOT_CELL;
    bin_t cur_bin = m_root_bin;

    for( ;; ) {
        if( bin == cur_bin )
            break;

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

    assert( cur_bin.layer_bits() > BITMAP_LAYER_BITS );

    /* Proccess common case */
    if( bin.layer_bits() > BITMAP_LAYER_BITS ) {
        if( bin == cur_bin )
            return get_fill_type(m_cell[cur_cell].m_left.m_bitmap, m_cell[cur_cell].m_right.m_bitmap);
        if( bin < cur_bin )
            return get_fill_type(m_cell[cur_cell].m_left.m_bitmap);
        return get_fill_type(m_cell[cur_cell].m_right.m_bitmap);
    }

    /* Process low-layers case */
    assert( bin != cur_bin );

    const bitmap_t bm1 = BITMAP[ BITMAP_LAYER_BITS & static_cast<uint64_t>(bin) ];
    const bitmap_t bm2 = (bin < cur_bin) ? m_cell[cur_cell].m_left.m_bitmap : m_cell[cur_cell].m_right.m_bitmap;

    const bitmap_t bitmap = bm1 & bm2;

    if( bitmap == BITMAP_EMPTY )
        return FILL_EMPTY;
    if( bitmap == bm1 )
        return FILL_FILLED;
    return FILL_MIXED;
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

    /* Store the trace history */
    ref_t _trace_cell[64];
    ref_t * trace_cell = _trace_cell;

    /* Process first stage -- do not touch existed tree */
    ref_t cur_cell = ROOT_CELL;
    bin_t cur_bin = m_root_bin;

    *trace_cell++ = ROOT_CELL;
    while( cur_bin != bin ) {
        if( bin < cur_bin ) {
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


        assert( trace_cell < _trace_cell + sizeof(_trace_cell) / sizeof(_trace_cell[0]) );
        *trace_cell++ = cur_cell;
    }

    assert( cur_bin.layer_bits() > BITMAP_LAYER_BITS );

    /* If the bin cell was found */
    if( cur_bin == bin ) {  /* special */
        if( is_ref_left(cur_bin) ) {
            free_cell(m_cell[cur_bin].m_left.m_ref);
            set_bitmap_left(cur_bin);
        }
        if( is_ref_right(cur_bin) ) {
            free_cell(m_cell[cur_bin].m_right.m_ref);
            set_bitmap_right(cur_bin);
        }

        m_cell[cur_bin].m_left.m_bitmap = BITMAP_FILLED;
        m_cell[cur_bin].m_right.m_bitmap = BITMAP_FILLED;

        pack_cells(trace_cell - 1);

        return;
    }

    /* Process second stage */

    /* Gets the bin bitmap type */
    const int bin_bitmap_idx = static_cast<uint64_t>(bin) & BITMAP_LAYER_BITS;
    const bitmap_t bin_bitmap = BITMAP[ bin_bitmap_idx ] /* special */;

    /* Otherwise checking, are we need to do anything? */
    if( bin < cur_bin ) {
        if( (m_cell[cur_cell].m_left.m_bitmap & bin_bitmap) == bin_bitmap ) /* special */
            return;
    } else {
        if( (m_cell[cur_cell].m_right.m_bitmap & bin_bitmap) == bin_bitmap ) /* special */
            return;
    }

    /* Get the pre-bin */
    bin_t pre_bin = bin.parent();   /* OPTIMIZE IT! */
    while( pre_bin.layer_bits() <= BITMAP_LAYER_BITS )
        pre_bin = pre_bin.parent();

    /* Continue to trace -- unpack the tree if needed */
    while( cur_bin != pre_bin ) {
        if( pre_bin < cur_bin ) {
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

    assert( cur_bin == pre_bin );
    assert( cur_bin.layer_bits() > BITMAP_LAYER_BITS );

    /* Complete setting */
    if( bin < cur_bin )
        m_cell[cur_cell].m_left.m_bitmap |= bin_bitmap; /* special */
    else
        m_cell[cur_cell].m_right.m_bitmap |= bin_bitmap; /* special */

    pack_cells(trace_cell - 1); /* FIXME: Some times this step is unnecessary */
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
    printf("bitmap:\n");

    for(int i = 0; i < 16; ++i) {
        for(int j = 0; j < 64; ++j)
            printf("%d", get(bin_t(i * 64 + j)));
        printf("\n");
    }

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
