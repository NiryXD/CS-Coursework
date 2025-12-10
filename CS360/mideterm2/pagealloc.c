/*
 * pagealloc.c
 *
 * Simple page allocator implemented on top of an anonymous mmap() region.
 * It manages a pool of fixed-size 4KB pages. A small bookkeeping area at
 * the start of the pool stores two 1-bit flags per page (packed into
 * bytes): a "taken" bit and a "last" bit which marks the final page of
 * a multi-page allocation.
 *
 * The implementation is intentionally compact and uses bit-twiddling to
 * track allocations. The comments below explain the layout and each
 * function in plain language for newcomers.
 *
 * NOTE: Although this file is about memory/page allocation, you asked for
 * an explanation of exec-family functions as well. There is no exec call
 * in this file, but the block below explains exec semantics in general
 * (useful when studying process control and IPC alongside allocators).
 */

#include "pagealloc.h"
#include <math.h>
#include <sys/mman.h>

/* Global state for the allocator */
size_t NUM_PAGES = 0;         /* number of allocator-managed pages */
void *pool_start = NULL;      /* start address returned by mmap */
size_t TOTAL_BYTES = 0;       /* total bytes allocated from mmap */
size_t bookkeeping_pages = 0; /* how many pages reserved for bookkeeping */

/* Quick primer: exec-family functions (execl, execv, execvp, execlp, etc.)
 * - These functions replace the current process image with a new program.
 * - They do NOT create a new process; they transform the calling process
 *   into a different program. To run another program while keeping the
 *   current one, you normally fork() first and call exec() in the child.
 * - Example: fork() -> child calls execv("/bin/ls", args) -> child now
 *   runs ls. The parent can wait() for the child's exit status.
 */

/* Helper functions to manipulate the packed bookkeeping bits.
 * Each byte tracks flags for 4 pages (2 bits per page):
 *   bit 7 = "taken" for page 0, bit 6 = "last" for page 0
 *   bit 5 = "taken" for page 1, bit 4 = "last" for page 1
 *   bit 3 = "taken" for page 2, bit 2 = "last" for page 2
 *   bit 1 = "taken" for page 3, bit 0 = "last" for page 3
 */
bool test_bit(char bitset, int index) {
    return (bitset >> index) & 1;
}

void set_bit(char *bitset, int index) {
    *bitset |= 1 << index;
}

void clear_bit(char *bitset, int index) {
    *bitset &= ~(1 << index);
}

/* Given a page number, compute which bookkeeping byte holds its bits and
 * the indices inside that byte for the "taken" and "last" flags.
 */
void get_bit_pos(size_t page, int *byte_index, int *taken_bit, int *last_bit){
    *byte_index = page / 4;          /* 4 pages share one bookkeeping byte */
    int offset = page % 4;
    *taken_bit = 7 - 2 * offset;    /* mapping into bits within the byte */
    *last_bit = 6 - 2 * offset;
}

/* Initialize allocator with a number of pages. Returns true on success.
 * The allocator reserves a small number of pages for bookkeeping and
 * mmaps the whole pool as one anonymous memory region that the program
 * can then parcel out as page-sized blocks.
 */
bool page_init(size_t pages){
    size_t max_pages = pow(2, 18); /* arbitrary safety cap */
    if (pages > max_pages || pages < 2) return false;

    /* bookkeeping_bytes: two bits per page, rounded up to whole bytes */
    size_t bookkeeping_bytes = (pages + 3) / 4;
    bookkeeping_pages = (bookkeeping_bytes + 4096 - 1) / 4096; /* bytes->pages */
    TOTAL_BYTES = (pages + bookkeeping_pages) * 4096;

    /* mmap a contiguous region: bookkeeping area at the front, then data */
    pool_start = mmap(NULL, TOTAL_BYTES, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (pool_start == MAP_FAILED) return false;

    NUM_PAGES = pages;

    return true;
}

/* Release the mmap'ed pool */
void page_deinit(void){
    munmap(pool_start, TOTAL_BYTES);
    NUM_PAGES = 0;
    pool_start = NULL;
}

/* Allocate a contiguous run of pages. The allocator scans the bookkeeping
 * area looking for `pages` free pages in a row. When found it marks the
 * "taken" bits for each page and sets the "last" bit on the final page
 * so page_free can know where the allocation ends.
 *
 * The returned pointer points to the first user page (after bookkeeping pages).
 */
void *page_alloc(size_t pages){
    if (pages <= 0) return NULL;

    size_t consec_pages = 0;
    size_t start_page = 0;

    char *bit = (char *)pool_start; /* bookkeeping begins at pool_start */

    for (size_t i = 0; i < NUM_PAGES; ++i){
        int byte_index, taken_bit, last_bit;
        get_bit_pos(i, &byte_index, &taken_bit, &last_bit);

        if (!test_bit(bit[byte_index], taken_bit)){
            /* this page is free */
            if (consec_pages == 0) start_page = i;
            consec_pages++;

            if (consec_pages == pages){
                /* mark all pages as taken and clear any previous "last" */
                for (size_t j = start_page; j < start_page + pages; ++j){
                    int b, t, l;
                    get_bit_pos(j, &b, &t, &l);
                    set_bit(&bit[b], t);   /* set taken */
                    clear_bit(&bit[b], l); /* clear last */
                }
                /* mark the final page's "last" bit so freeing knows where to stop */
                int b, t, l;
                get_bit_pos(start_page + pages - 1, &b, &t, &l);
                set_bit(&bit[b], l);

                /* return pointer to first data page (skip bookkeeping pages) */
                return bit + 4096 * (start_page + bookkeeping_pages);
            }
        } else {
            consec_pages = 0; /* hit a used page; reset scan */
        }
    }

    /* no large enough free run found */
    return NULL;
}

/* Free a previously allocated page run. The addr is mapped back to a
 * page index (subtract bookkeeping area) and then the allocator clears
 * taken bits until it finds the "last" marker.
 */
void page_free(void *addr){
    if (addr == NULL) return;

    char *bit = (char *)pool_start;

    size_t page = (addr - pool_start) / 4096 - bookkeeping_pages;
    if (page >= NUM_PAGES) return;

    while (page < NUM_PAGES){
        int byte_index, taken_bit, last_bit;
        get_bit_pos(page, &byte_index, &taken_bit, &last_bit);
        if (!test_bit(bit[byte_index], taken_bit)){
            break; /* already free or inconsistent state */
        }

        /* clear the taken bit and check if this was the last page */
        clear_bit(&bit[byte_index], taken_bit);
        int last = test_bit(bit[byte_index], last_bit);
        clear_bit(&bit[byte_index], last_bit);

        if (last) break; /* allocation ended */
        page++;
    }
}

/* Count how many individual pages are marked taken. */
size_t pages_taken(void){
    size_t count = 0;
    char *bit = (char *)pool_start;

    for (size_t i = 0; i < NUM_PAGES; ++i){
        int byte_index, taken_bit, last_bit;
        get_bit_pos(i, &byte_index, &taken_bit, &last_bit);
        if (test_bit(bit[byte_index], taken_bit)){
            count++;
        }
    }
    return count;
}

size_t pages_free(void){
    return NUM_PAGES - pages_taken();
}