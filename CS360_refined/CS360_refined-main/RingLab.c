/* 
 * Program Name: Ring Lab
 * Student Name: Ar-Raniry Ar-Rasyid  
 * Net ID: jzr266  
 * Student ID: 000-663-921  
 * Program Description: Circular Storage
 */

#include "ring.h"
#include <string.h>
#include <stdlib.h>

// Write your ring buffer functions here

struct RingBuffer
{
    size_t at;
    size_t size;
    size_t capacity;
    char *buffer;
};

RingBuffer *rb_new(size_t capacity) {
    RingBuffer *rb = (RingBuffer *)malloc(sizeof(RingBuffer));
    if (!rb)
          return NULL;

    if (capacity == 0) {
        rb->buffer = NULL;
    }
    else {
        rb->buffer = (char *)calloc(capacity, sizeof(char));
        if (rb->buffer == NULL) {
            free(rb);
            return NULL;
        }
}
    
    rb->at = 0;
    rb->size = 0;
    rb->capacity = capacity;
    return rb;
}

void rb_free ( RingBuffer *rb) {
    free(rb->buffer);
    free(rb);
}

size_t rb_at (const RingBuffer *rb) {
     return rb->at;
}

size_t rb_size(const RingBuffer *rb) {
    return  rb->size;
}

size_t rb_capacity(const RingBuffer *rb){
    return rb->capacity;
}

void rb_clear(RingBuffer *rb) {
    rb->at = 0;
    rb->size = 0;
}

bool    rb_push(RingBuffer *rb, char data) {
    size_t end;

    if (rb == NULL) {
        return false;
    }
    if (rb->size == rb->capacity) {
        return false;
    }
    if (rb->capacity == 0) {
        return false;
    }

    end = (rb->at + rb->size) % rb->capacity;
    rb->buffer[end] = data;
    rb->size = rb->size + 1;
    return true;
}

bool    rb_pop(RingBuffer *rb, char *data){
    if (rb == NULL) {
        return false;
    }
    if (rb->capacity == 0) {
        return false;
    }

    if (data != NULL) {
        *data = rb->buffer[rb->at];
    }
    rb->at = (rb->at + 1) % rb->capacity;
    rb->size = rb->size - 1;
    return true;

}

char    rb_peek(const RingBuffer *rb){
    return rb->buffer[rb->at];
}

void    rb_ignore(RingBuffer *rb, size_t num){
    size_t ignore;

    if (num < rb->size) {
        ignore = num;
    }
    else {
        ignore = rb->size;
    }

    rb->at = (rb->at + ignore) % rb->capacity;
    rb->size = rb->size - ignore;
}

size_t  rb_read(RingBuffer *rb, char *buf, size_t max_bytes){
    size_t count;
    size_t first;
    size_t remain;
    
    if (rb->size < max_bytes) {
        count = rb->size;
    }
    else {
        count = max_bytes;
    }

    if(count < (rb->capacity - rb->at)) {
        first = count;
    }
    else {
        first = rb->capacity - rb->at;
    }

    memcpy(buf, rb->buffer + rb->at, first);
    // I had ChatGPT explain how memcpy worked
    // https://www.geeksforgeeks.org/cpp/memcpy-in-cc/
    // I also used this site

    rb->at = rb->at + first;
    if (rb->at >= rb->capacity) {
        rb->at = rb->at - rb->capacity;
    }

    rb->size = rb->size - first;

    remain = count - first;
    if (remain > 0){
        memcpy(buf + first, rb->buffer, remain);

        rb->at = rb->at + remain;
        if (rb->at >= rb->capacity) {
            rb->at = rb->at - rb->capacity;
        }
        rb->size = rb->size -  remain;
    }

    return count; 
}

size_t  rb_write(RingBuffer *rb, const char *buf, size_t bytes) {
    size_t remain;
    size_t first;
    size_t end;
    size_t count;
    size_t available;

    available = rb->capacity - rb->size;
    if (available == 0) {
        return 0;
    } 
     

    if (bytes < available) {
        count = bytes;
    }
    else {
        count = available;
    }

    end = (rb->at + rb->size) % rb->capacity;

    if (count < (rb->capacity - end)){
        first = count;
    }
    else {
        first = rb->capacity - end;
    }

    memcpy(rb->buffer + end, buf, first);
    rb->size = rb->size - first;

    remain = count -  first; 
    if (remain > 0){
        memcpy(rb->buffer, buf + first, remain);
        rb->size = rb->size + remain;
    }
    return count;
}
