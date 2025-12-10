/* 
 * Program Name: Vector Lab 
 * Student Name: Ar-Raniry Ar-Rasyid
 * Net ID: jzr266
 * Student ID: 000-663-921
 * Program Description: Breathes life into functions
 *
 * What this file does:
 *   Implements a growable array of 64-bit integers called Vector.
 *   It starts small, and when you add more items than it can hold,
 *   it requests more heap memory and keeps going.
 *
 * Key ideas in plain language:
 *   - size: how many items are actually stored right now
 *   - capacity: how many items we can hold without growing
 *   - values: pointer to a heap array of int64_t
 *
 * Memory functions used:
 *   - malloc(N): get N bytes from the heap once
 *   - realloc(ptr, N): resize the existing allocation to N bytes, moving it if needed
 *   - free(ptr): give the heap memory back when done
 *
 * Sorting and searching:
 *   - vector_sort uses qsort with a comparator to order items ascending
 *   - vector_bsearch does a binary search on a sorted vector
 *   - vector_find does a linear scan on an unsorted vector
 */

#include "vector.h"
#include <stdlib.h>
#include <search.h> // not strictly needed for qsort; qsort is in stdlib.h

// The actual structure that backs a Vector.
// Users only see "Vector" from the header; fields are hidden here.
struct Vector {
    size_t   size;      // number of elements currently stored
    size_t   capacity;  // total slots available before growing
    int64_t *values;    // pointer to the heap array of int64_t
};

Vector *vector_new(void) {
    // Allocate the Vector object itself.
    Vector *vectoor = (Vector *)malloc(sizeof(Vector));
    // If malloc fails, it returns NULL. Always check before using the pointer.
    if(!vectoor) {
        return NULL;
    }

    // Start empty
    vectoor->size = 0;
    vectoor->capacity = 0;

    // Start with a zero-length allocation.
    // Note: malloc(0) is allowed but implementation defined. It may return NULL or a unique pointer.
    // Your code defends against NULL by trying malloc(1) next.
    vectoor->values = (int64_t *)malloc(0);
    if (!vectoor->values) {
        // Fallback to allocate 1 byte if malloc(0) yields NULL.
        // This makes values non-NULL so later realloc has something to work with.
        vectoor->values = (int64_t *)malloc(1);
    }

    return vectoor;
}

Vector *vector_new_with_capacity(size_t capacity) {
    // Allocate the Vector object itself.
    Vector *vectoor = (Vector *)malloc(sizeof(Vector));
    if(!vectoor) {
        return NULL;
    }

    vectoor->size = 0;
    vectoor->capacity = capacity;

    // Allocate space for exactly capacity int64_t items.
    vectoor->values = (int64_t *)malloc(capacity * sizeof(int64_t));
    // If malloc fails, values will be NULL. Caller should check returned Vector if desired.
    return vectoor;
}

void vector_free(Vector *vec) {
    if (!vec) {
        return;
    }
    // Free the item array first, then the struct itself.
    free(vec->values);
    free(vec);
}

// Change logical size to new_size and grow capacity if needed.
// If new_size is larger than capacity, we realloc the array to fit.
// New slots are uninitialized.
void vector_resize(Vector *vec, size_t new_size) {
    if (!vec) {
        return;
    }

    if (new_size > vec->capacity) {
        // Ask the heap to resize the memory block to hold new_size items.
        int64_t *nValues = (int64_t *)realloc(vec->values, new_size * sizeof(int64_t));
        if (!nValues) {
            // If realloc fails, keep the old memory and do not change size.
            return;
        }
        vec->values = nValues;
        vec->capacity = new_size; // capacity grows to match requested size
    }
    vec->size = new_size; // logical size is whatever caller asked for
}

// Append one value at the end.
// If full, grow the vector by calling resize to size+1.
void vector_push(Vector *vec, int64_t value) {
    if (!vec) {
        return;
    }

    if (vec->size == vec->capacity){
        // This grows by exactly 1. Simple, but can cause many reallocs.
        // Alternative: grow by a factor (like 2x) to reduce realloc frequency.
        vector_resize(vec, vec->size + 1);
    } else {
        vec->size += 1;
    }

    vec->values[vec->size - 1] = value;
}

// Insert value at index, shifting items right.
// If index is past the end, just push at the end.
void vector_insert(Vector *vec, size_t index, int64_t value){
    if (!vec) {
        return;
    }

    if (index >= vec->size) {
        vector_push(vec, value);
        return;
    }

    if (vec->size == vec->capacity) {
        vector_resize(vec, vec->size + 1);
    } else {
        vec->size += 1;
    }

    // Shift items right to open a hole at index.
    for (size_t i = vec->size - 1; i > index; i--) {
        vec->values[i] = vec->values[i-1];
    }
    vec->values[index] = value;
}

// Remove the item at index by shifting items left.
// Return true on success, false on bad index.
bool vector_remove(Vector *vec, size_t index) {
    if (!vec || index >= vec->size) {
        return false;
    }

    for (size_t i = index; i + 1 < vec->size; ++i) {
        vec->values[i] = vec->values[i + 1];
    }

    vec->size -= 1;
    return true;
}

// Shrink capacity down to match current size to save memory.
// If size is zero, this may reduce to a 0 or 1 byte allocation per your fallback policy.
void vector_shrink(Vector *vec) {
    if (!vec) {
        return;
    }
    if (vec->capacity == vec->size) {
        return; // already as small as possible
    }

    size_t nSize = vec->size;
    size_t nMem  = nSize * sizeof(int64_t);

    int64_t *nPtr = (int64_t *)realloc(vec->values, nMem);
    if (!nPtr) {
        if (nMem == 0) {
            // If we tried to realloc to 0 and got NULL, use your 0-or-1 policy.
            nPtr = (int64_t *)malloc(0);
            if (!nPtr) {
                nPtr = (int64_t *)malloc(1);
            }
        } else {
            return; // keep old allocation if shrink failed
        }
    }
    vec->values   = nPtr;
    vec->capacity = nSize;
}

// Safe get with bounds check. Writes the value into *value if provided.
bool vector_get(const Vector *vec, size_t index, int64_t *value){
    if (!vec || index >= vec->size) {
        return false;
    }
    if (value) {
        *value = vec->values[index];
    }
    return true;
}

// Fast get without bounds checks. Caller must ensure index < size.
int64_t vector_get_unchecked(const Vector *vec, size_t index) {
    return vec->values[index];
}

// Safe set with bounds check.
bool vector_set(Vector *vec, size_t index, int64_t value){
    if (!vec || index >= vec->size) {
        return false;
    }
    vec->values[index] = value;
    return true;
}

// Comparator for ascending order used by qsort.
// qsort passes pointers to elements, so we cast and dereference.
// Return negative if a < b, positive if a > b, zero if equal.
int comp_ascending(const void *left, const void *right) {
    int64_t a = *(const int64_t *)left;
    int64_t b = *(const int64_t *)right;

    if (a <  b) return -1;
    if (a >  b) return  1;
    return 0;
}

// Convenience: sort ascending using our comparator.
void vector_sort(Vector *vec) {
    vector_sort_by(vec, comp_ascending);
}

// Sort with a caller-supplied comparator function.
// qsort(base, count, elem_size, compare_fn)
// compare_fn takes two const void* to elements and returns negative, zero, or positive.
void vector_sort_by(Vector *vec, SortFunc comp) {
    if (!vec || vec->size == 0) {
        return;
    }
    qsort(vec->values, vec->size, sizeof(int64_t), comp);
}

// Binary search for value in a sorted vector (ascending).
// Returns index if found, otherwise -1.
ssize_t vector_bsearch(const Vector *vec, int64_t value){
    if (!vec || vec->size == 0)
        return -1;

    size_t left  = 0;
    size_t right = vec->size; // half-open interval [left, right)

    while (left < right) {
        size_t  middle      = left + (right - left) / 2;
        int64_t middleValue = vec->values[middle];

        if (middleValue == value) {
            return (ssize_t)middle;
        } else if (middleValue < value) {
            left = middle + 1;  // go right
        } else {
            right = middle;     // go left
        }
    }
    return -1;
}

// Linear scan for value. Works on unsorted vectors.
// Returns index if found, otherwise -1.
ssize_t vector_find(const Vector *vec, int64_t value) {
    if (!vec || vec->size == 0) {
        return -1;
    }

    for (size_t i = 0; i < vec->size; i++) {
        if (vec->values[i] == value) {
            return (ssize_t)i;
        }
    }
    return -1;
}

// Ensure capacity is at least new_capacity. Never reduces capacity.
void vector_reserve(Vector *vec, size_t new_capacity) {
    if (!vec)
        return;

    // Never reserve less than current size, or we would lose data.
    if (new_capacity < vec->size) {
        new_capacity = vec->size;
    }

    if (new_capacity > vec->capacity) {
        int64_t *nValues = (int64_t *)realloc(vec->values, new_capacity * sizeof(int64_t));
        if (!nValues) {
            return; // keep old storage if grow failed
        }
        vec->values   = nValues;
        vec->capacity = new_capacity;
    }
}

// Keep allocation but forget contents by setting size to 0.
void vector_clear(Vector *vec){
    if (!vec)
        return;
    vec->size = 0;
}

// Report capacity
size_t vector_capacity(const Vector *vec) {
    if (!vec)
        return 0;
    return vec->capacity;
}

// Report size
size_t vector_size(const Vector *vec){
    if (!vec)
        return 0;
    return vec->size;
}
