/* 
 * Program Name: Vector Lab 
 * Student Name: Ar-Raniry Ar-Rasyid  
 * Net ID: jzr266  
 * Student ID: 000-663-921  
 * Program Description: Breathes life into functions
 */

#include "vector.h"
#include <stdlib.h>
#include <search.h>

// Write your vector functions here.
// int main() is written in main.c
// You can navigate files to the left of this window.

struct Vector {
size_t size;
size_t capacity;
int64_t *values;
};

Vector *vector_new(void) {
    Vector *vectoor = (Vector *)malloc(sizeof(Vector));
    if(!vectoor) {
        return NULL;
    }

    vectoor->size = 0;
    vectoor->capacity = 0;

    vectoor->values = (int64_t *)malloc(0);
    if (!vectoor->values) {
        vectoor->values = (int64_t *)malloc(1);
    }

    return vectoor;
}

Vector *vector_new_with_capacity(size_t capacity) {
    Vector *vectoor = (Vector *)malloc(sizeof(Vector));
    if(!vectoor) {
        return NULL;
    }

    vectoor->size = 0;
    vectoor->capacity = capacity;
    vectoor->values = (int64_t *)malloc(capacity * sizeof(int64_t));

    return vectoor;
}

void vector_free(Vector *vec) {
    if (!vec) {
        return ;
    }

    free(vec->values);
    free(vec);
}

void vector_resize(Vector *vec, size_t new_size) {
        if (!vec) {
        return ;
    }

    if (new_size > vec->capacity) {
        int64_t *nValues = (int64_t *)realloc(vec->values, new_size * sizeof(int64_t));
        if (!nValues) {
            return;
        }

        vec->values = nValues;
        vec->capacity = new_size;
    }
    vec->size = new_size;
}

void vector_push(Vector *vec, int64_t value) {
    if (!vec) {
        return ;
    }

    if (vec->size == vec->capacity){
        vector_resize(vec, vec->size + 1);
    }
    else {
        vec->size += 1;
    }
    vec->values[vec->size - 1] = value;
}

void vector_insert(Vector *vec, size_t index, int64_t value){
    if (!vec) {
        return ;
    }

    if (index >= vec->size) {
        vector_push(vec, value);
        return;
    }

    if (vec->size == vec->capacity) {
        vector_resize(vec, vec->size + 1);
    }
    else {
        vec->size += 1;
    }

    for (size_t i = vec->size - 1; i > index; i--) {
        vec->values[i] = vec->values[i-1];
    }
    vec->values[index] = value;

}

bool vector_remove(Vector *vec, size_t index) {
    if (!vec || index >= vec-> size) {
        return false;
    }

    for (size_t i = index; i + 1 < vec->size; ++i) {
        vec->values[i] = vec->values[i + 1];
    }

    vec->size -= 1;
    return true;
}

void vector_shrink(Vector *vec) {
    if (!vec) {
        return ;
    }

    if (vec->capacity == vec->size) {
        return;
    }

    size_t nSize = vec->size;
    size_t nMem = nSize * sizeof(int64_t);

    int64_t *nPtr = (int64_t *)realloc(vec->values, nMem);
    if (!nPtr) {
        if (nMem == 0) {
            nPtr = (int64_t *)malloc(0);
            if (!nPtr) {
                nPtr = (int64_t *)malloc(1);
            }
        }
        else {
            return;
        }
    }
vec->values = nPtr;
vec->capacity = nSize;
}

bool vector_get(const Vector *vec, size_t index, int64_t *value){
        if (!vec || index >= vec-> size) {
        return false;
    }

    if (value) {
        *value = vec->values[index];
    }
    return true;
}

int64_t vector_get_unchecked(const Vector *vec, size_t index) {
    return vec->values[index];
}

bool vector_set(Vector *vec, size_t index, int64_t value){
        if (!vec || index >= vec-> size) {
        return false;
    }

    vec->values[index] = value;
    return true;
}

int comp_ascending(const void *left, const void *right) {
    int64_t a = *(int64_t *)left;
     int64_t b = *(int64_t *)right;   

     if (a < b)
     return -1;
     if (b < a) 
     return 1;
    return 0;
}

void vector_sort(Vector *vec) 
{
    vector_sort_by(vec, comp_ascending);
}

void vector_sort_by(Vector *vec, SortFunc comp) {
    if (!vec || vec->size == 0) 
    return;
    qsort(vec->values, vec->size, sizeof(int64_t), comp);
}

ssize_t vector_bsearch(const Vector *vec, int64_t value){
    if (!vec || vec->size == 0)
    return -1;

    size_t left = 0;
    size_t right = vec->size;

    while (left < right) {
        size_t middle = left + (right - left) /  2;
        int64_t middleValue = vec->values[middle];

        if (middleValue == value) {
            return (ssize_t)middle;
        }
        else if (middleValue < value) {
            left = middle + 1;
        }
        else {
            right = middle;
        }
    }
    return -1;
}

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

void vector_reserve(Vector *vec, size_t new_capacity) {
    if (!vec)
    return;

    if (new_capacity < vec->size) {
        new_capacity = vec->size;
    }

    if (new_capacity > vec->capacity) {
        int64_t *nValues = (int64_t *)realloc(vec->values, new_capacity * sizeof(int64_t));
        if (!nValues) {
            return;
        }
        vec->values = nValues;
        vec->capacity = new_capacity;
    }
}

void vector_clear(Vector *vec){
    if (!vec)
    return;

    vec->size = 0;
}

size_t vector_capacity(const Vector *vec) {
    if (!vec)
    return 0;

    return vec->capacity;

}

size_t vector_size(const Vector *vec){

    if (!vec)
    return 0;

    return vec->size;


}


