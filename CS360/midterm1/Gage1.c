#include "pagealloc.h"
#include <math.h>
#include <sys/mman.h>

size_t NUM_PAGES = 0;
void *pool_start = NULL;
size_t TOTAL_BYTES = 0;
size_t bookkeeping_pages = 0;

bool test_bit(char bitset, int index) {
    return (bitset >> index) & 1;
}

void set_bit(char *bitset, int index) {
    *bitset |= 1 << index;
}

void clear_bit(char *bitset, int index) {
    *bitset &= ~(1 << index);
}

void get_bit_pos(size_t page, int *byte_index, int *taken_bit, int *last_bit){
    *byte_index = page / 4;
    int offset = page % 4;
    *taken_bit = 7 - 2 * offset;
    *last_bit = 6 - 2 * offset;
}

bool page_init(size_t pages){
    size_t max_pages = pow(2, 18);
    if (pages > max_pages || pages < 2) return false;

    size_t bookkeeping_bytes = (pages + 3) / 4;
    bookkeeping_pages = (bookkeeping_bytes + 4096 - 1) / 4096;
    TOTAL_BYTES = (pages + bookkeeping_pages) * 4096;

    pool_start = mmap(NULL, TOTAL_BYTES, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (pool_start == MAP_FAILED) return false;

    NUM_PAGES = pages;

    return true;
}

void page_deinit(void){
    munmap(pool_start, TOTAL_BYTES);
    NUM_PAGES = 0;
    pool_start = NULL;
}

void *page_alloc(size_t pages){
    if (pages <= 0) return NULL;

    size_t consec_pages = 0;
    size_t start_page = 0;

    char *bit = (char *)pool_start;

    for (size_t i = 0; i < NUM_PAGES; ++i){
        int byte_index, taken_bit, last_bit;
        get_bit_pos(i, &byte_index, &taken_bit, &last_bit);

        if (!test_bit(bit[byte_index], taken_bit)){
            if (consec_pages == 0) start_page = i;
            consec_pages++;

            if (consec_pages == pages){
                for (size_t j = start_page; j < start_page + pages; ++j){
                    int b, t, l;
                    get_bit_pos(j, &b, &t, &l);
                    set_bit(&bit[b], t);
                    clear_bit(&bit[b], l);
                }
                int b, t, l;
                get_bit_pos(start_page + pages - 1, &b, &t, &l);
                set_bit(&bit[b], l);

                return bit + 4096 * (start_page + bookkeeping_pages);
            }
        } else {
            consec_pages = 0;
        }
    }

    return NULL;
}

void page_free(void *addr){
    if (addr == NULL) return;

    char *bit = (char *)pool_start;

    size_t page = (addr - pool_start) / 4096 - bookkeeping_pages;
    if (page >= NUM_PAGES) return;

    while (page < NUM_PAGES){
        int byte_index, taken_bit, last_bit;
        get_bit_pos(page, &byte_index, &taken_bit, &last_bit);
        if (!test_bit(bit[byte_index], taken_bit)){
            break;
        }

        clear_bit(&bit[byte_index], taken_bit);
        int last = test_bit(bit[byte_index], last_bit);
        clear_bit(&bit[byte_index], last_bit);

        if (last) break;
        page++;
    }
}

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