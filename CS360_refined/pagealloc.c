#include "pagealloc.h"
#include <sys/mman.h>
#include <string.h>
#include <stddef.h>

static const size_t PageSize =  4096u;
// Establish one page is 4096 unsigned bytes
static const size_t MaxData = (1u << 18);
// Max Pages

// Globals

static void *Base = NULL;
static size_t DataPageCount = 0;
static size_t BookPageCount = 0;
static size_t sizeBytes = 0;
static unsigned char *BookkeepingBase = NULL;

static int TestBit(unsigned char bitset, int index ) {
    return (bitset >> index) & 1u;
}

static void SetBit(unsigned char *bitset, int index) {
 *bitset |= (unsigned char)(1u << index);
}

static void ClearBit (unsigned char *bitset, int index) {
    *bitset &= (unsigned char)~(unsigned char)(1u << index);
}
// I kept getting warnings

// Write your page allocator functions here.

bool page_init(size_t pages){
    if (pages < 2 || pages > MaxData)
    return false;

    if (Base) {
        munmap(Base, sizeBytes);
        Base = NULL;
    }

    // figure out how many bytes needed
    size_t bookKBytes = (pages + 3) / 4;
    size_t bookKAligned = ((bookKBytes + PageSize - 1 ) / PageSize) * PageSize;
    size_t bookKPages = bookKAligned / PageSize;
    size_t tBytes = (pages + bookKPages) * PageSize;
    // I had Chat put the write up put in more layman terms

    void *pool = mmap(NULL, tBytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    // Ripped from power points
    if (pool == MAP_FAILED)
    return false;

    Base = pool;
    DataPageCount = pages;
    BookPageCount = bookKPages;
    sizeBytes = tBytes;
    BookkeepingBase = (unsigned char *)Base;

    return true;



}
void page_deinit(void){
    if (!Base)
    return;

    munmap(Base, sizeBytes);
    Base = NULL;
    DataPageCount = 0;
BookPageCount = 0;
sizeBytes = 0;
BookkeepingBase = NULL;

}

void *page_alloc(size_t pages){
    if (!Base || pages == 0 || pages > DataPageCount)
    return NULL;

    size_t length = 0;
    size_t start = 0;
    int found = 0;
// search for not taken pages
    for (size_t i = 0; i < DataPageCount; i++) {
        unsigned char *b = BookkeepingBase + (i/4);
        int Pair = 2 * (3 - (int)(i % 4));
        int Taken = Pair + 1;

        if(!TestBit(*b, Taken)) {
            if (length == 0)
            start = i;
            length++;

            if (length == pages) {
                found = 1;
                break;
            }
            }
            else {
                length = 0;
            }
        }
        if (!found)
        return NULL;

        for (size_t i = 0; i < pages; i++) {
            size_t index = start + i;
            unsigned char *b = BookkeepingBase + (index /4);
            int Pair = 2 * (3 - (index % 4));
            int Taken = Pair + 1;
            int Last = Pair + 0;

            SetBit(b, Taken);
            if (i + 1 == pages)
            SetBit(b, Last);
            else 
            ClearBit(b, Last);


        }
        char *DataStart = (char *)Base + BookPageCount * PageSize;
        return (void *)(DataStart + start * PageSize);
    }


void page_free(void *addr) {
    if (!Base || !addr)
    return;

    size_t page = (size_t)(((char*)addr - (char*)Base) / (ptrdiff_t)PageSize);
    // https://en.cppreference.com/w/c/types/ptrdiff_t.html
    if (page < BookPageCount)
    return;
    page -= BookPageCount;
    if (page >= DataPageCount)
    return;
    // clear pages until "last"
    for (size_t p = page; p < DataPageCount; p++) {
        unsigned char *b = BookkeepingBase + (p / 4);
        int Pair = 2 * (3 - (p % 4));
        int Taken = Pair + 1;
        int Last = Pair + 0;

        if (!TestBit(*b, Taken))
        break;

        int WasLast = TestBit(*b, Last);
        ClearBit(b, Taken);
        ClearBit(b, Last);
        if (WasLast)
        break;
    }
}

size_t pages_taken(void){
    if (!Base)
    return 0;

    size_t count = 0;
    for (size_t i = 0; i < DataPageCount; i++) {
        unsigned char *b = BookkeepingBase + (i/4);
        int Pair = 2 * (3 - (i % 4));
        int Taken = Pair + 1;
        if (TestBit(*b, Taken)) 
        count++;
    }
    return count;
}
size_t pages_free(void){
        if (!Base)
    return 0;

    return DataPageCount - pages_taken();

}
