#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstdarg>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <cstring>


unsigned long int s_block_count;
unsigned long int s_block_bitmap_size;
char * s_block_bitmap;

unsigned long int s_count;


FILE * s_fin;
FILE * s_fout;
const char * s_finname;
const char * s_foutname;

const size_t S_LINE_SIZE = 1024 * 1024;
char s_line[S_LINE_SIZE];


void usage() {
    fprintf(stderr, "usage: [dump-file [bitmap-file]]\n");
    exit(-1);
}


static inline void p_error(const char * format, ...) {
    va_list ap;

    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    exit(-1);
}


static inline bool next_line(bool eof_expected = false) {
    fgets(s_line, S_LINE_SIZE, s_fin);

    if( ferror(s_fin) )
        p_error("%s: %d: %s\n", s_finname, errno, strerror(errno));

    if( !eof_expected && feof(s_fin) )
        p_error("%s: -1: Unexpected end of file\n", s_finname);

    return !feof(s_fin);
}


static inline bool str_begins_with(register const char * nptr, register const char * prefix) {
    while( *nptr != '\0' && *nptr == *prefix ) {
        ++nptr;
        ++prefix;
    }

    return *prefix == '\0';
}


static inline const char * str_skip_spaces(register const char * nptr) {
    while( isspace(*nptr) )
        ++nptr;

    return nptr;
}


static inline const char * str_skip_chr(const char * nptr, char chr) {
    if( *nptr != chr )
        p_error("%s: -1: Pattern not found.\n", s_finname);
    return nptr + 1;
}


static inline const char * str_to_num(const char * nptr, unsigned long int & x) {
    nptr = str_skip_spaces(nptr);
    if( '-' == *nptr )
        p_error("%s: -1: Positive value is expected.\n", s_finname);

    const char * endptr = nptr;
    errno = 0;

    x = strtoul(nptr, const_cast<char **>(&endptr), 0);
    if( errno != 0 )
        p_error("%s: %d: %s\n", s_finname, errno, strerror(errno));
    if( endptr == nptr )
        p_error("%s: -1: Numerical value is expected.\n", s_finname);

    return endptr;
}


static inline bool str_is_empty(const char * nptr) {
    return '\0' == *str_skip_spaces(nptr);
}


static inline const char * str_str_end(const char * nptr, const char * pat) {
    const char * ptr = strstr(nptr, pat);
    if( ptr == NULL )
        p_error("%s: -1: Pattern not found.\n", s_finname);
    return ptr + strlen(pat);
}


void process_header() {
    while( next_line() && !str_begins_with(s_line, "Block count:") );

    str_to_num(strchr(s_line, ':') + 1, s_block_count);

    while( next_line() && !str_is_empty(s_line) );
    while( next_line() && str_is_empty(s_line) );


    s_block_bitmap_size = (s_block_count + 7) / 8;
    s_block_bitmap = new char[s_block_bitmap_size];

    memset(s_block_bitmap, 0xff, s_block_bitmap_size);
}


void process_group_header(unsigned long int & group_begin, unsigned long int & group_end, bool & is_block_uninit) {
    const char * str = str_str_end(s_line, "Blocks ");

    str = str_to_num(str, group_begin);
    str = str_skip_chr(str, '-');
    str = str_to_num(str, group_end);

    if( group_end < group_end )
        p_error("%s: -1: group_end < group_end\n", s_finname);
    if( s_block_count < group_end )
        p_error("%s: -1: s_block_count < group_end\n", s_finname);

    is_block_uninit = (NULL != strstr(str, "BLOCK_UNINIT"));
}


bool process_group() {
    if( !str_begins_with(s_line, "Group ") )
        return false;

//    fprintf(stderr, "%s", s_line);

    unsigned long int group_begin;
    unsigned long int group_end;
    bool is_block_uninit;

    process_group_header(group_begin, group_end, is_block_uninit);

    if( is_block_uninit ) {
        for(unsigned long int n = group_begin; n <= group_end; ++n)
            s_block_bitmap[n / 8] &= ~(1 << (n % 8));

        s_count += group_end + 1 - group_begin;   /* -- */

        while( next_line(true) && str_begins_with(s_line, "  ") );


    } else {
        while( next_line(true) && str_begins_with(s_line, "  ") ) {
            if( !str_begins_with(s_line, "  Free blocks:") )
                continue;

            const char * str = str_skip_spaces(strchr(s_line, ':') + 1);

            while( *str != '\0' ) {
                unsigned long int begin, end;

                str = str_to_num(str, begin);
                if( *str == '-' )
                    str = str_to_num(str + 1, end);
                else
                    end = begin;

                if( end < begin || begin < group_begin || group_end < end )
                    p_error("%s: -1: Block range overflow.\n", s_finname);

                for(unsigned long int n = begin; n <= end; ++n)
                    s_block_bitmap[n / 8] &= ~(1 << (n % 8));

                s_count += end + 1 - begin; /* -- */

                if( *str == ',' )
                    ++str;
                str = str_skip_spaces(str);
            }
        }
    }

    return true;
}


int main(int argc, char ** argv) {
    if( 3 < argc )
        usage();

    s_finname = "stdin";
    s_foutname = "stdout";

    s_fin = stdin;
    s_fout = stdout;

    if( argc > 1 ) {
        s_finname = argv[1];
        s_fin = fopen(s_finname, "rb");

        if( s_fin == NULL )
            p_error("%s: %d: %s\n", s_finname, errno, strerror(errno));
    }

    process_header();
    while( process_group() );

    if( argc > 2 ) {
        s_foutname = argv[2];
        s_fout = fopen(s_foutname, "wb");

        if( s_fout == NULL )
            p_error("%s: %d: %s\n", s_foutname, errno, strerror(errno));
    }

    if( 1 != fwrite(s_block_bitmap, s_block_bitmap_size, 1, s_fout) )
        p_error("%s: %d: %s\n", s_foutname, errno, strerror(errno));


    fprintf(stderr, "Count: %lu\n", s_count);
    fprintf(stderr, "Filling: %.2f\n", 100.0 - 100.0 * s_count / s_block_count);

    return 0;
}
