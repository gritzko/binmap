#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "binmap.h"

/* Constants */
const bitmap_t BITMAP_EMPTY  = static_cast<bitmap_t>(0);
const bitmap_t BITMAP_FILLED = static_cast<bitmap_t>(-1);

static const size_t ROOT_REF = 0;

static const bin_t::uint_t BITMAP_LAYER_BITS = 2 * 8 * sizeof(bitmap_t) - 1;

static const bitmap_t BITMAP[] = {
    static_cast<bitmap_t>(0x00000001), static_cast<bitmap_t>(0x00000003),
    static_cast<bitmap_t>(0x00000002), static_cast<bitmap_t>(0x0000000f),
    static_cast<bitmap_t>(0x00000004), static_cast<bitmap_t>(0x0000000c),
    static_cast<bitmap_t>(0x00000008), static_cast<bitmap_t>(0x000000ff),
    static_cast<bitmap_t>(0x00000010), static_cast<bitmap_t>(0x00000030),
    static_cast<bitmap_t>(0x00000020), static_cast<bitmap_t>(0x000000f0),
    static_cast<bitmap_t>(0x00000040), static_cast<bitmap_t>(0x000000c0),
    static_cast<bitmap_t>(0x00000080), static_cast<bitmap_t>(0x0000ffff),
    static_cast<bitmap_t>(0x00000100), static_cast<bitmap_t>(0x00000300),
    static_cast<bitmap_t>(0x00000200), static_cast<bitmap_t>(0x00000f00),
    static_cast<bitmap_t>(0x00000400), static_cast<bitmap_t>(0x00000c00),
    static_cast<bitmap_t>(0x00000800), static_cast<bitmap_t>(0x0000ff00),
    static_cast<bitmap_t>(0x00001000), static_cast<bitmap_t>(0x00003000),
    static_cast<bitmap_t>(0x00002000), static_cast<bitmap_t>(0x0000f000),
    static_cast<bitmap_t>(0x00004000), static_cast<bitmap_t>(0x0000c000),
    static_cast<bitmap_t>(0x00008000), static_cast<bitmap_t>(0xffffffff),
    static_cast<bitmap_t>(0x00010000), static_cast<bitmap_t>(0x00030000),
    static_cast<bitmap_t>(0x00020000), static_cast<bitmap_t>(0x000f0000),
    static_cast<bitmap_t>(0x00040000), static_cast<bitmap_t>(0x000c0000),
    static_cast<bitmap_t>(0x00080000), static_cast<bitmap_t>(0x00ff0000),
    static_cast<bitmap_t>(0x00100000), static_cast<bitmap_t>(0x00300000),
    static_cast<bitmap_t>(0x00200000), static_cast<bitmap_t>(0x00f00000),
    static_cast<bitmap_t>(0x00400000), static_cast<bitmap_t>(0x00c00000),
    static_cast<bitmap_t>(0x00800000), static_cast<bitmap_t>(0xffff0000),
    static_cast<bitmap_t>(0x01000000), static_cast<bitmap_t>(0x03000000),
    static_cast<bitmap_t>(0x02000000), static_cast<bitmap_t>(0x0f000000),
    static_cast<bitmap_t>(0x04000000), static_cast<bitmap_t>(0x0c000000),
    static_cast<bitmap_t>(0x08000000), static_cast<bitmap_t>(0xff000000),
    static_cast<bitmap_t>(0x10000000), static_cast<bitmap_t>(0x30000000),
    static_cast<bitmap_t>(0x20000000), static_cast<bitmap_t>(0xf0000000),
    static_cast<bitmap_t>(0x40000000), static_cast<bitmap_t>(0xc0000000),
    static_cast<bitmap_t>(0x80000000), /* special */ static_cast<bitmap_t>(0xffffffff) /* special */
};


/**
 * Get the leftmost bin that coresponded to bitmap (the bin is filled in bitmap)
 */
static inline bin_t::uint_t bitmap_to_bin(register bitmap_t b) {
    static const unsigned char BITMAP_TO_BIN[] = {
        -1, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
         8, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
        10, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
         9, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
        12, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
         8, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
        10, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
         9, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
        14, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
         8, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
        10, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
         9, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
        13, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
         8, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
        10, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
        11, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 7
    };

    assert( sizeof(bitmap_t) <= 4 );
    assert( b != BITMAP_EMPTY );

    unsigned char t;

    t = BITMAP_TO_BIN[ b & 0xff ];
    if( t < 16 ) {
        if( t != 7 )
            return static_cast<bin_t::uint_t>(t);

        b += 1;
        b &= -b;
        if( !b )
            return BITMAP_LAYER_BITS / 2;
        if( b >= 0x10000 )
            return 15;
        return 7;
    }

    b >>= 8;
    t = BITMAP_TO_BIN[ b & 0xff ];
    if( t <= 15 )
            return 16 + t;

    /* Recursion */
    // return 32 + bitmap_to_bin( b >> 16 );

    assert( sizeof(bitmap_t) == 4 );

    b >>= 8;
    t = BITMAP_TO_BIN[ b & 0xff ];
    if( t < 16 ) {
        if( t != 7 )
            return 32 + static_cast<bin_t::uint_t>(t);

        b += 1;
        b &= -b;
        if( b >= 0x10000 )
            return 47;
        return 39;
    }

    return 48 + BITMAP_TO_BIN[ b >> 8 ];}


/* Methods */

/**
 * Constructor
 */
binmap_t::binmap_t() : m_root_bin(63) {
    assert( sizeof(bitmap_t) <= 4 );

    m_cell = NULL;
    m_blocks_number = 0;
    m_cells_number = 0;
    m_free_top = ROOT_REF;

    const ref_t ROOT_REF = alloc_cell();

    assert( ROOT_REF == 0 && m_blocks_number > 0 );
}


/**
 * Destructor
 */
binmap_t::~binmap_t() {
    if( m_cell )
        free(m_cell);
}


/**
 * Allocates one cell
 */
ref_t binmap_t::alloc_cell() {
    if( m_free_top == ROOT_REF ) {
        /* Check for reference capacity */
        if( static_cast<ref_t>(16 * m_blocks_number) < 16 * m_blocks_number ) {
            fprintf(stderr, "Warning: binmap_t::alloc_cell: REFERENCE LIMIT ERROR\n");
            return ROOT_REF /* REFERENCE LIMIT ERROR */;
        }

        /* Extend the buffer */
        const size_t old_size = m_blocks_number;
        const size_t new_size = (old_size ? 2 * old_size : 1);

        const size_t size1 = 16 * new_size * sizeof(m_cell[0]);

        /* Check for integer overflow */
        if( size1 == 0 ) {
            fprintf(stderr, "Warning: binmap_t::alloc_cell: INTEGER OVERFLOW\n");
            return ROOT_REF /* INTEGER OVERFLOW */;
        }

        /* Reallocate memory */
        cell_t * const cell = static_cast<cell_t *>(realloc(m_cell, size1));
        if( cell == NULL ) {
            fprintf(stderr, "Warning: binmap_t::alloc_cell: MEMORY ERROR\n");
            return ROOT_REF /* MEMORY ERROR */;
        }

        m_cell = cell;
        m_blocks_number = new_size;

        /* Insert new cells to the free cell list */
        const size_t stop_idx = 16 * old_size - 1;
        size_t idx = 16 * new_size - 1;

        m_cell[ idx ].m_is_free = true;
        m_cell[ idx ].m_free_next = m_free_top;

        for(--idx; idx != stop_idx; --idx) {
            m_cell[ idx ].m_is_free = true;
            m_cell[ idx ].m_free_next = static_cast<ref_t>(idx + 1);
        }

        m_free_top = static_cast<ref_t>(16 * old_size);
    }

    /* Pop an element from the free cell list */
    const ref_t ref = m_free_top;
    assert( m_cell[ref].m_is_free );

    m_free_top = m_cell[ ref ].m_free_next;

    /* Clean it */
    memset(&m_cell[ref], 0, sizeof(m_cell[ref]));

    ++m_cells_number;

    return ref;
}


/**
 * Releases the cell
 */
void binmap_t::free_cell(ref_t ref) {
    assert( ref > 0 );
    assert( !m_cell[ref].m_is_free );

    if( m_cell[ref].m_is_left_ref )
        free_cell(m_cell[ref].m_left.m_ref);
    if( m_cell[ref].m_is_right_ref )
        free_cell(m_cell[ref].m_right.m_ref);

    m_cell[ref].m_is_free = true;
    m_cell[ref].m_free_next = m_free_top;
    m_free_top = ref;

    --m_cells_number;
}


/**
 * Extend root
 */
void binmap_t::extend_root() {
    assert( m_root_bin != bin_t::ALL );

    if( m_cell[ROOT_REF].m_left.m_bitmap == m_cell[ROOT_REF].m_right.m_bitmap ) {
        /* Setup the root cell */
        m_cell[ROOT_REF].m_right.m_bitmap = BITMAP_EMPTY;

    } else {
        /* Allocate new cell */
        const ref_t ref = alloc_cell();
        if( ref == ROOT_REF )
            return /* ALLOC ERROR */;

        /* Move old root to the cell */
        m_cell[ref] = m_cell[ROOT_REF];

        /* Setup new root */
        m_cell[ROOT_REF].m_is_left_ref = true;
        m_cell[ROOT_REF].m_is_right_ref = false;

        m_cell[ROOT_REF].m_left.m_ref = ref;
        m_cell[ROOT_REF].m_right.m_bitmap = BITMAP_EMPTY;
    }

    /* Reset bin */
    m_root_bin = m_root_bin.parent();
}


/**
 * Unpack the left half of a cell
 */
ref_t binmap_t::unpack_left_half(ref_t ref) {
    assert( !m_cell[ref].m_is_left_ref );

    const ref_t left_ref = alloc_cell();
    if( left_ref == ROOT_REF )
        return ROOT_REF /* ALLOC ERROR */;

    m_cell[left_ref].m_left.m_bitmap = m_cell[ref].m_left.m_bitmap;
    m_cell[left_ref].m_right.m_bitmap = m_cell[ref].m_left.m_bitmap;

    m_cell[ref].m_is_left_ref = true;
    m_cell[ref].m_left.m_ref = left_ref;

    return left_ref;
}


/**
 * Unpack the right half of a cell
 */
ref_t binmap_t::unpack_right_half(ref_t ref) {
    assert( !m_cell[ref].m_is_right_ref );

    const ref_t right_ref = alloc_cell();
    if( right_ref == ROOT_REF )
        return ROOT_REF /* ALLOC ERROR */;

    m_cell[right_ref].m_left.m_bitmap = m_cell[ref].m_right.m_bitmap;
    m_cell[right_ref].m_right.m_bitmap = m_cell[ref].m_right.m_bitmap;

    m_cell[ref].m_is_right_ref = true;
    m_cell[ref].m_right.m_ref = right_ref;

    return right_ref;
}


/**
 * Pack a trace of cells
 */
void binmap_t::pack_cells(ref_t * trace_ref) {
    ref_t ref = *trace_ref--;
    if( ref == ROOT_REF )
        return;

    assert( !m_cell[ref].m_is_left_ref && !m_cell[ref].m_is_right_ref );

    if( m_cell[ref].m_left.m_bitmap != m_cell[ref].m_right.m_bitmap )
        return;

    const bitmap_t bitmap = m_cell[ref].m_left.m_bitmap;

    do {
        ref = *trace_ref--;

        if( !m_cell[ref].m_is_left_ref ) {
            if( m_cell[ref].m_left.m_bitmap != bitmap )
                break;

        } else if( !m_cell[ref].m_is_right_ref ) {
            if( m_cell[ref].m_right.m_bitmap != bitmap )
                break;

        } else
            break;

    } while( ref != ROOT_REF );

    const ref_t par_ref = trace_ref[2];

    if( m_cell[ref].m_is_left_ref && m_cell[ref].m_left.m_ref == par_ref ) {
        m_cell[ref].m_is_left_ref = false;
        m_cell[ref].m_left.m_bitmap = bitmap;
    } else {
        m_cell[ref].m_is_right_ref = false;
        m_cell[ref].m_right.m_bitmap = bitmap;
    }

    free_cell(par_ref);
}


/**
 * Get bins
 *
 * @param bin
 *             the bin
 * @return fill type of the bin
 */
bool binmap_t::get(bin_t bin) const {
    if( !m_root_bin.contains(bin) )
        return false;

    /* Trace the bin */
    ref_t cur_ref = ROOT_REF;
    bin_t cur_bin = m_root_bin;

    for( ;; ) {
        if( bin == cur_bin )
            break;

        if( bin < cur_bin ) {
            if( m_cell[cur_ref].m_is_left_ref ) {
                cur_ref = m_cell[cur_ref].m_left.m_ref;
                cur_bin.to_left();
            } else
                break;
        } else {
            if( m_cell[cur_ref].m_is_right_ref ) {
                cur_ref = m_cell[cur_ref].m_right.m_ref;
                cur_bin.to_right();
            } else
                break;
        }
    }

    assert( cur_bin.layer_bits() > BITMAP_LAYER_BITS );

    /* Proccess common case */
    if( bin.layer_bits() > BITMAP_LAYER_BITS ) {
        if( bin == cur_bin )
            return m_cell[cur_ref].m_left.m_bitmap == BITMAP_FILLED && m_cell[cur_ref].m_right.m_bitmap == BITMAP_FILLED;
        if( bin < cur_bin )
            return m_cell[cur_ref].m_left.m_bitmap == BITMAP_FILLED;
        return m_cell[cur_ref].m_right.m_bitmap == BITMAP_FILLED;
    }

    /* Process low-layers case */
    assert( bin != cur_bin );

    const bitmap_t bm1 = BITMAP[ BITMAP_LAYER_BITS & bin.toUInt() ];
    const bitmap_t bm2 = (bin < cur_bin) ? m_cell[cur_ref].m_left.m_bitmap : m_cell[cur_ref].m_right.m_bitmap;

    return (bm1 & bm2) == bm1;
}


/**
 * Find first empty bin
 */
bin_t binmap_t::find_empty() const {
    /* Trace the bin */
    bitmap_t bitmap = BITMAP_FILLED;

    ref_t cur_ref = ROOT_REF;
    bin_t cur_bin = m_root_bin;

//    if( m_cell[cur_ref].m_left.m_bitmap == BITMAP_EMPTY && m_cell[cur_ref].m_right.m_bitmap == BITMAP_EMPTY )
//        return bin_t::ALL;

    for( ;; ) {
        if( m_cell[cur_ref].m_is_left_ref ) {
            cur_ref = m_cell[cur_ref].m_left.m_ref;
            cur_bin.to_left();
        } else if( m_cell[cur_ref].m_left.m_bitmap != BITMAP_FILLED ) {
            bitmap = m_cell[cur_ref].m_left.m_bitmap;
            cur_bin.to_left();
            break;
        } else if( m_cell[cur_ref].m_is_right_ref ) {
            cur_ref = m_cell[cur_ref].m_right.m_ref;
            cur_bin.to_right();
        } else {
            bitmap = m_cell[cur_ref].m_right.m_bitmap;
            cur_bin.to_right();
            break;
        }
    }

    if( bitmap == BITMAP_FILLED ) {
        if( m_root_bin.is_all() )
            return bin_t::NONE;
        return m_root_bin.sibling();
    }

    return bin_t(cur_bin.base_left().toUInt() + bitmap_to_bin(~bitmap));
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
    ref_t _trace_ref[64];
    ref_t * trace_ref = _trace_ref;

    /* Process first stage -- do not touch existed tree */
    ref_t cur_ref = ROOT_REF;
    bin_t cur_bin = m_root_bin;

    *trace_ref++ = ROOT_REF;
    while( cur_bin != bin ) {
        if( bin < cur_bin ) {
            if( m_cell[cur_ref].m_is_left_ref ) {
                cur_ref = m_cell[cur_ref].m_left.m_ref;
                cur_bin.to_left();
            } else
                break;
        } else {
            if( m_cell[cur_ref].m_is_right_ref ) {
               cur_ref = m_cell[cur_ref].m_right.m_ref;
               cur_bin.to_right();
            } else
                break;
        }

        assert( trace_ref < _trace_ref + sizeof(_trace_ref) / sizeof(_trace_ref[0]) );
        *trace_ref++ = cur_ref;
    }

    assert( cur_bin.layer_bits() > BITMAP_LAYER_BITS );

    /* If the bin cell was found */
    if( cur_bin == bin ) {  /* special */
        if( m_cell[cur_ref].m_is_left_ref )
            free_cell(m_cell[cur_ref].m_left.m_ref);
        if( m_cell[cur_ref].m_is_right_ref )
            free_cell(m_cell[cur_ref].m_right.m_ref);

        m_cell[cur_ref].m_is_left_ref = false;
        m_cell[cur_ref].m_is_right_ref = false;

        m_cell[cur_ref].m_left.m_bitmap = BITMAP_FILLED;
        m_cell[cur_ref].m_right.m_bitmap = BITMAP_FILLED;

        pack_cells(trace_ref - 1);

        return;
    }

    /* Process second stage */

    /* Gets the bin bitmap type */
    const int bin_bitmap_idx = bin.toUInt() & BITMAP_LAYER_BITS;
    const bitmap_t bin_bitmap = BITMAP[ bin_bitmap_idx ] /* special */;

    /* Otherwise checking, are we need to do anything? */
    if( bin < cur_bin ) {
        if( (m_cell[cur_ref].m_left.m_bitmap & bin_bitmap) == bin_bitmap ) /* special */
            return;
    } else {
        if( (m_cell[cur_ref].m_right.m_bitmap & bin_bitmap) == bin_bitmap ) /* special */
            return;
    }

    /* Get the pre-bin */
    bin_t pre_bin = bin.parent();   /* OPTIMIZE IT! */
    while( pre_bin.layer_bits() <= BITMAP_LAYER_BITS )
        pre_bin = pre_bin.parent();

    /* Continue to trace -- unpack the tree if needed */
    while( cur_bin != pre_bin ) {
        if( pre_bin < cur_bin ) {
            cur_ref = unpack_left_half(cur_ref);
            cur_bin.to_left();
        } else {
            cur_ref = unpack_right_half(cur_ref);
            cur_bin.to_right();
        }

        if( cur_ref == ROOT_REF ) {
            pack_cells(trace_ref - 1);
            return; /* UNPACK HALF ERROR */
        }

        *trace_ref++ = cur_ref;
    }

    assert( cur_bin == pre_bin );
    assert( cur_bin.layer_bits() > BITMAP_LAYER_BITS );

    /* Complete setting */
    if( bin < cur_bin )
        m_cell[cur_ref].m_left.m_bitmap |= bin_bitmap; /* special */
    else
        m_cell[cur_ref].m_right.m_bitmap |= bin_bitmap; /* special */

    pack_cells(trace_ref - 1); /* FIXME: Some times this step is unnecessary */
}


/**
 * Resets bins
 *
 * @param bin
 *             the bin
 */
void binmap_t::reset(bin_t bin) {
    if( bin.is_none() )
        return;

    /* Extending binmap if needed */
    while( !m_root_bin.contains(bin) )
        extend_root();

    /* Store the trace history */
    ref_t _trace_ref[64];
    ref_t * trace_ref = _trace_ref;

    /* Process first stage -- do not touch existed tree */
    ref_t cur_ref = ROOT_REF;
    bin_t cur_bin = m_root_bin;

    *trace_ref++ = ROOT_REF;
    while( cur_bin != bin ) {
        if( bin < cur_bin ) {
            if( m_cell[cur_ref].m_is_left_ref ) {
                cur_ref = m_cell[cur_ref].m_left.m_ref;
                cur_bin.to_left();
            } else
                break;
        } else {
            if( m_cell[cur_ref].m_is_right_ref ) {
               cur_ref = m_cell[cur_ref].m_right.m_ref;
               cur_bin.to_right();
            } else
                break;
        }

        assert( trace_ref < _trace_ref + sizeof(_trace_ref) / sizeof(_trace_ref[0]) );
        *trace_ref++ = cur_ref;
    }

    assert( cur_bin.layer_bits() > BITMAP_LAYER_BITS );

    /* If the bin cell was found */
    if( cur_bin == bin ) {  /* special */
        if( m_cell[cur_ref].m_is_left_ref )
            free_cell(m_cell[cur_ref].m_left.m_ref);
        if( m_cell[cur_ref].m_is_right_ref )
            free_cell(m_cell[cur_ref].m_right.m_ref);

        m_cell[cur_ref].m_is_left_ref = false;
        m_cell[cur_ref].m_is_right_ref = false;

        m_cell[cur_ref].m_left.m_bitmap = BITMAP_EMPTY;
        m_cell[cur_ref].m_right.m_bitmap = BITMAP_EMPTY;

        pack_cells(trace_ref - 1);

        return;
    }

    /* Process second stage */

    /* Gets the bin bitmap type */
    const int bin_bitmap_idx = bin.toUInt() & BITMAP_LAYER_BITS;
    const bitmap_t bin_bitmap = BITMAP[ bin_bitmap_idx ] /* special */;

    /* Otherwise checking, are we need to do anything? */
    if( bin < cur_bin ) {
        if( (m_cell[cur_ref].m_left.m_bitmap & bin_bitmap) == 0 ) /* special */
            return;
    } else {
        if( (m_cell[cur_ref].m_right.m_bitmap & bin_bitmap) == 0 ) /* special */
            return;
    }

    /* Get the pre-bin */
    bin_t pre_bin = bin.parent();   /* OPTIMIZE IT! */
    while( pre_bin.layer_bits() <= BITMAP_LAYER_BITS )
        pre_bin = pre_bin.parent();

    /* Continue to trace -- unpack the tree if needed */
    while( cur_bin != pre_bin ) {
        if( pre_bin < cur_bin ) {
            cur_ref = unpack_left_half(cur_ref);
            cur_bin.to_left();
        } else {
            cur_ref = unpack_right_half(cur_ref);
            cur_bin.to_right();
        }

        if( cur_ref == ROOT_REF ) {
            pack_cells(trace_ref - 1);
            return; /* UNPACK HALF ERROR */
        }

        *trace_ref++ = cur_ref;
    }

    assert( cur_bin == pre_bin );
    assert( cur_bin.layer_bits() > BITMAP_LAYER_BITS );

    /* Complete setting */
    if( bin < cur_bin )
        m_cell[cur_ref].m_left.m_bitmap &= ~bin_bitmap; /* special */
    else
        m_cell[cur_ref].m_right.m_bitmap &= ~bin_bitmap; /* special */

    pack_cells(trace_ref - 1); /* FIXME: Some times this step is unnecessary */
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

    printf("size: %u bytes\n", sizeof(*this) + (16 * sizeof(cell_t)) * blocks_number());
    printf("cells number: %u (of %u)\n", cells_number(), 16 * blocks_number());
    printf("root bin: %llu\n", static_cast<unsigned long long>(m_root_bin.toUInt()));

//    ref_t _nil_cell[256];
//    bin_t _nil_bin[256];
//    ref_t * top_cell = _nil_cell;
//    bin_t * top_bin = _nil_bin;
//
//    *top_cell++ = ROOT_REF;
//    *top_bin++ = m_root_bin;
//
//    while( top_cell != _nil_cell ) {
//        const ref_t cell = *(--top_cell);
//        const bin_t bin = *(--top_bin);
//
//        printf("Cell: %u, Bin: %llu, Layer: %d\n", cell, static_cast<unsigned long long>(bin.toUInt()), bin.layer());
//
//        if( is_ref_left(cell) ) {
//            const ref_t left_ref = m_cell[cell].m_left.m_ref;
//            printf(" left ref: %u, bin: %llu\n", left_ref, static_cast<unsigned long long>(bin.left().toUInt()));
//            *top_cell++ = left_ref;
//            *top_bin++ = bin.left();
//        } else {
//            const bitmap_t left_bitmap = m_cell[cell].m_left.m_bitmap;
//            printf(" left bitmap: %08x\n", left_bitmap);
//        }
//
//        if( is_ref_right(cell) ) {
//            const ref_t right_ref = m_cell[cell].m_right.m_ref;
//            printf(" right ref: %u, bin: %llu\n", right_ref, static_cast<unsigned long long>(bin.right().toUInt()));
//            *top_cell++ = right_ref;
//            *top_bin++ = bin.right();
//        } else {
//            const bitmap_t right_bitmap = m_cell[cell].m_right.m_bitmap;
//            printf(" right bitmap: %08x\n", right_bitmap);
//        }
//    }
}
