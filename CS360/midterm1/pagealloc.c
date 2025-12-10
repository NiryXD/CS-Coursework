// pagealloc.c
// Uses only what is in your slides: mmap/munmap, bit ops, no stdint.h, no preprocessor conditionals.

#include "pagealloc.h"
#include <sys/mman.h>   // mmap, munmap

// ---------------- constants ----------------
//#define PAGE_SIZE 4096UL
//#define MAX_DATA_PAGES (1u << 18)  // 262,144 data pages (excludes bookkeeping)

static const size_t PAGE_SIZE       = 4096u;
static const size_t MAX_DATA_PAGES  = (1u << 18);

// ---------------- globals ------------------
static void   *g_pool        = NULL;   // start of entire mmap region
static size_t  g_data_pages  = 0;      // pages available to hand out
static size_t  g_book_pages  = 0;      // pages reserved for bookkeeping
static size_t  g_total_bytes = 0;      // total bytes mapped
static unsigned char *g_book_ptr = NULL; // start of bookkeeping bytes

// --------- bookkeeping bit helpers --------
// Two bits per page in each bookkeeping byte, high to low for pages 0..3:
//   page 0 -> bits 7:6  (taken:7, last:6)
//   page 1 -> bits 5:4  (taken:5, last:4)
//   page 2 -> bits 3:2  (taken:3, last:2)
//   page 3 -> bits 1:0  (taken:1, last:0)

static inline unsigned char *bk_byte(size_t p) {
    return g_book_ptr + (p / 4);
}

static inline int bk_pair_base(size_t p) {
    int pair = (int)(p % 4);
    return 2 * (3 - pair); // 6,4,2,0 for pages 0,1,2,3
}

static inline int bit_taken_idx(size_t p) { return bk_pair_base(p) + 1; } // left bit
static inline int bit_last_idx (size_t p) { return bk_pair_base(p) + 0; } // right bit

static inline int test_bit(unsigned char byte, int idx)    { return (byte >> idx) & 1; }
static inline void set_bit (unsigned char *byte, int idx)  { *byte |=  (unsigned char)(1u << idx); }
static inline void clear_bit(unsigned char *byte, int idx) { *byte &= (unsigned char)~(1u << idx); }

static inline int is_taken(size_t p) {
    unsigned char *b = bk_byte(p);
    return test_bit(*b, bit_taken_idx(p));
}
static inline int is_last(size_t p) {
    unsigned char *b = bk_byte(p);
    return test_bit(*b, bit_last_idx(p));
}
static inline void mark_taken(size_t p, int on) {
    unsigned char *b = bk_byte(p);
    if (on) set_bit(b, bit_taken_idx(p)); else clear_bit(b, bit_taken_idx(p));
}
static inline void mark_last(size_t p, int on) {
    unsigned char *b = bk_byte(p);
    if (on) set_bit(b, bit_last_idx(p)); else clear_bit(b, bit_last_idx(p));
}

// -------- address <-> page helpers --------
// Use char* arithmetic only (no stdint.h)
static inline void *page_to_ptr(size_t p) {
    return (void *)((char *)g_pool + p * PAGE_SIZE);
}
static inline int ptr_to_page(void *addr, size_t *out_p) {
    if (!g_pool || !addr) return 0;
    char *base = (char *)g_pool;
    char *a    = (char *)addr;
    char *end  = base + g_data_pages * PAGE_SIZE; // only data region is valid
    if (a < base || a >= end) return 0;
    size_t diff = (size_t)(a - base);
    if ((diff % PAGE_SIZE) != 0) return 0; // must be page-aligned
    *out_p = diff / PAGE_SIZE;
    return 1;
}

// ---------------- API ---------------------

bool page_init(size_t pages) {
    // Reject out-of-range requests on data pages only
    if (pages < 2 || pages > MAX_DATA_PAGES) return false;

    // If already initialized, tear down first
    if (g_pool) {
        munmap(g_pool, g_total_bytes);
        g_pool = NULL;
    }

    // Two bits per page -> 4 pages per bookkeeping byte
    size_t bk_bytes = (pages + 3) / 4; // ceil(pages/4)

    // Align bookkeeping bytes up to a whole page
    size_t bk_bytes_aligned = (bk_bytes + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;
    size_t bk_pages = bk_bytes_aligned / PAGE_SIZE;

    // Total mapping includes data pages + bookkeeping pages
    size_t total_bytes = (pages + bk_pages) * PAGE_SIZE;

    void *pool = mmap(NULL, total_bytes, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (pool == MAP_FAILED) {
        g_pool = NULL;
        return false;
    }

    g_pool        = pool;
    g_data_pages  = pages;
    g_book_pages  = bk_pages;
    g_total_bytes = total_bytes;

    // Bookkeeping lives at the top of the mapping (after data pages)
    g_book_ptr = (unsigned char *)((char *)g_pool + g_data_pages * PAGE_SIZE);

    // Clear bookkeeping without memset
    for (size_t i = 0; i < bk_bytes_aligned; ++i) g_book_ptr[i] = 0;

    return true;
}

void page_deinit(void) {
    if (!g_pool) return;
    munmap(g_pool, g_total_bytes);
    g_pool        = NULL;
    g_data_pages  = 0;
    g_book_pages  = 0;
    g_total_bytes = 0;
    g_book_ptr    = NULL;
}

void *page_alloc(size_t pages) {
    if (!g_pool || pages == 0 || pages > g_data_pages) return NULL;

    // Find a contiguous run of free pages of the requested length
    size_t run_len = 0;
    size_t run_start = 0;
    int found = 0;

    for (size_t i = 0; i < g_data_pages; ++i) {
        if (!is_taken(i)) {
            if (run_len == 0) run_start = i;
            run_len++;
            if (run_len == pages) { found = 1; break; }
        } else {
            run_len = 0;
        }
    }

    if (!found) return NULL;

    // Mark pages as taken; set last only on the final page
    for (size_t j = 0; j < pages; ++j) {
        size_t p = run_start + j;
        mark_taken(p, 1);
        mark_last(p, 0);
    }
    mark_last(run_start + pages - 1, 1);

    return page_to_ptr(run_start);
}

void page_free(void *addr) {
    if (!g_pool || !addr) return;

    size_t p0;
    if (!ptr_to_page(addr, &p0)) return;  // not within data region or not aligned
    if (!is_taken(p0)) return;            // not allocated

    // Ensure this is the start of an allocation
    if (p0 > 0 && is_taken(p0 - 1) && !is_last(p0 - 1)) {
        // Pointer is into the middle of a block; ignore
        return;
    }

    // Free forward until (and including) the page whose last bit is set
    for (size_t p = p0; p < g_data_pages; ++p) {
        int was_last = is_last(p);
        mark_taken(p, 0);
        mark_last(p, 0);
        if (was_last) break;
    }
}

size_t pages_taken(void) {
    if (!g_pool) return 0;
    size_t count = 0;
    for (size_t p = 0; p < g_data_pages; ++p) {
        if (is_taken(p)) count++;
    }
    return count;
}

size_t pages_free(void) {
    if (!g_pool) return 0;
    size_t count = 0;
    for (size_t p = 0; p < g_data_pages; ++p) {
        if (!is_taken(p)) count++;
    }
    return count;
}
