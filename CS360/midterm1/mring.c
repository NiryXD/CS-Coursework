/* 
 * Program Name: Ring Lab
 * Student Name: Ar-Raniry Ar-Rasyid
 * Net ID: jzr266
 * Student ID: 000-663-921
 * Program Description: Circular Storage
 *
 * What this file does:
 *   Implements a ring buffer (circular buffer) for bytes.
 *   A ring buffer uses a fixed-size array to store data in FIFO order.
 *   When you reach the end of the array, you wrap around to the front.
 *
 * Core ideas in plain language:
 *   - capacity: how many bytes we can hold total
 *   - size: how many bytes are currently stored
 *   - at: index where the next read happens (the head)
 *   - write position: computed as (at + size) % capacity
 *
 * Operations:
 *   - rb_new(cap): allocate the buffer structure and its array
 *   - rb_free: free memory
 *   - rb_at / rb_size / rb_capacity: getters
 *   - rb_clear: make the buffer empty
 *   - rb_push: add one byte at the tail if there is room
 *   - rb_pop: remove one byte from the head if there is data
 *   - rb_peek: look at the next byte without removing it
 *   - rb_ignore: skip N bytes from the head
 *   - rb_read: copy up to N bytes out into a user buffer
 *   - rb_write: copy up to N bytes from a user buffer into the ring
 *
 * Notes:
 *   - All functions are single-threaded. For multithreading, add locks.
 *   - Reading from an empty buffer fails. Writing to a full buffer writes nothing.
 */

#include "ring.h"
#include <string.h>
#include <stdlib.h>

struct RingBuffer {
    size_t at;        // index of the next byte to read (head)
    size_t size;      // number of bytes currently stored
    size_t capacity;  // total slots available
    char *buffer;     // allocated array of bytes
};

RingBuffer *rb_new(size_t capacity) {
    // Allocate the RingBuffer object itself
    RingBuffer *rb = (RingBuffer *)malloc(sizeof(RingBuffer));
    if (!rb) {
        return NULL; // allocation failed
    }

    // Allocate the data array if capacity > 0
    if (capacity == 0) {
        rb->buffer = NULL;
    } else {
        // calloc zeros memory; not required, but nice for cleanliness
        rb->buffer = (char *)calloc(capacity, sizeof(char));
        if (rb->buffer == NULL) {
            free(rb);
            return NULL;
        }
    }

    // Initialize fields
    rb->at = 0;
    rb->size = 0;
    rb->capacity = capacity;
    return rb;
}

void rb_free(RingBuffer *rb) {
    if (!rb) return;          // free(NULL) is safe, but this avoids double frees
    free(rb->buffer);         // free the data array first
    free(rb);                 // then free the struct
}

size_t rb_at(const RingBuffer *rb) {
    return rb ? rb->at : 0;
}

size_t rb_size(const RingBuffer *rb) {
    return rb ? rb->size : 0;
}

size_t rb_capacity(const RingBuffer *rb) {
    return rb ? rb->capacity : 0;
}

void rb_clear(RingBuffer *rb) {
    if (!rb) return;
    rb->at = 0;   // reset head
    rb->size = 0; // mark as empty
}

bool rb_push(RingBuffer *rb, char data) {
    if (rb == NULL) {
        return false;
    }
    if (rb->capacity == 0) {
        return false; // nowhere to put data
    }
    if (rb->size == rb->capacity) {
        return false; // buffer is full
    }

    // Tail position is head + size, wrapped by capacity
    size_t end = (rb->at + rb->size) % rb->capacity;

    // Write the byte and increase size
    rb->buffer[end] = data;
    rb->size += 1;
    return true;
}

bool rb_pop(RingBuffer *rb, char *data) {
    if (rb == NULL) {
        return false;
    }
    if (rb->capacity == 0) {
        return false;
    }
    if (rb->size == 0) {
        return false; // buffer is empty, nothing to pop
    }

    // Copy out the byte at the head if caller wants it
    if (data != NULL) {
        *data = rb->buffer[rb->at];
    }

    // Move head forward by one with wrap-around, and reduce size
    rb->at = (rb->at + 1) % rb->capacity;
    rb->size -= 1;
    return true;
}

char rb_peek(const RingBuffer *rb) {
    // Caller should ensure size > 0 before peeking
    // If empty, this will read invalid data; an alternative is to return '\0'
    return rb->buffer[rb->at];
}

void rb_ignore(RingBuffer *rb, size_t num) {
    if (!rb) return;
    if (rb->capacity == 0) return;

    // Only skip what we have
    size_t ignore = (num < rb->size) ? num : rb->size;

    // Advance head with wrap-around and reduce size
    rb->at = (rb->at + ignore) % rb->capacity;
    rb->size -= ignore;
}

size_t rb_read(RingBuffer *rb, char *buf, size_t max_bytes) {
    if (!rb || !buf) return 0;
    if (rb->capacity == 0) return 0;

    // We can read up to the minimum of (size, max_bytes)
    size_t count = (rb->size < max_bytes) ? rb->size : max_bytes;

    // First contiguous chunk from head to end of physical array
    size_t first = (count < (rb->capacity - rb->at)) ? count
                                                     : (rb->capacity - rb->at);

    // Copy first chunk
    memcpy(buf, rb->buffer + rb->at, first);

    // Advance head and reduce size
    rb->at += first;
    if (rb->at >= rb->capacity) {
        rb->at -= rb->capacity; // wrap-around
    }
    rb->size -= first;

    // If we still have more to read, it must wrap to index 0
    size_t remain = count - first;
    if (remain > 0) {
        memcpy(buf + first, rb->buffer, remain);

        rb->at += remain;
        if (rb->at >= rb->capacity) {
            rb->at -= rb->capacity;
        }
        rb->size -= remain;
    }

    return count; 
}

size_t rb_write(RingBuffer *rb, const char *buf, size_t bytes) {
    if (!rb || !buf) return 0;
    if (rb->capacity == 0) return 0;

    // Free space available right now
    size_t available = rb->capacity - rb->size;
    if (available == 0) {
        return 0; // full
    }

    // We can only write up to available
    size_t count = (bytes < available) ? bytes : available;

    // Tail position is head + size, wrapped by capacity
    size_t end = (rb->at + rb->size) % rb->capacity;

    // First contiguous chunk from end to physical end
    size_t first = (count < (rb->capacity - end)) ? count
                                                  : (rb->capacity - end);

    // Copy first chunk into the ring
    memcpy(rb->buffer + end, buf, first);

    // If there is a wrap-around, copy the remainder starting at index 0
    size_t remain = count - first;
    if (remain > 0) {
        memcpy(rb->buffer, buf + first, remain);
    }

    // Increase size by total bytes written
    rb->size += count;

    return count;
}
