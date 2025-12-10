#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "tpool.h"

// Simple executor for testing
static uint64_t square(int value) {
    return (uint64_t)(value * value);
}

// Slower executor to test thread pool efficiency
static uint64_t slow_executor(int value) {
    usleep(10000); // 10ms delay
    return (uint64_t)(value * 2);
}

// Create a test file with known thing
void create_test_file(const char *filename, const char *thing) {
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd >= 0) {
        write(fd, thing, strlen(thing));
        close(fd);
    }
}

void test_basic_functionality() {
    printf("\n=== Testing Basic Functionality ===\n");
    
    void *handle = thread_pool_open(4);
    if (handle == NULL) {
        printf("FAIL: Could not open thread pool\n");
        return;
    }
    printf("passes: Thread pool opened with 4 threads\n");
    
    const int values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const int num_values = sizeof(values) / sizeof(values[0]);
    
    uint64_t *result = thread_pool_execute(handle, values, num_values, square);
    if (result == NULL) {
        printf("FAIL: Could not execute on thread pool\n");
        thread_pool_close(handle);
        return;
    }
    
    int passes = 1;
    for (int i = 0; i < num_values; i++) {
        uint64_t expect = (uint64_t)(values[i] * values[i]);
        if (result[i] != expect) {
            printf("FAIL: Result %d = %lu, expect %lu\n", i, result[i], expect);
            passes = 0;
        }
    }
    
    if (passes) {
        printf("passes: All square calculations correct\n");
    }
    
    free(result);
    thread_pool_close(handle);
    printf("passes: Thread pool closed successfully\n");
}

void test_multiple_executions() {
    printf("\n=== Testing Multiple Executions ===\n");
    
    void *handle = thread_pool_open(8);
    if (handle == NULL) {
        printf("FAIL: Could not open thread pool\n");
        return;
    }
    
    // First execution
    const int values1[] = {1, 2, 3, 4, 5};
    uint64_t *results1 = thread_pool_execute(handle, values1, 5, square);
    
    // Second execution
    const int values2[] = {10, 20, 30};
    uint64_t *results2 = thread_pool_execute(handle, values2, 3, slow_executor);
    
    if (results1 && results2) {
        printf("passes: Multiple executions completed\n");
        
        // Verify first result
        int passes = 1;
        for (int i = 0; i < 5; i++) {
            if (results1[i] != (uint64_t)(values1[i] * values1[i])) {
                passes = 0;
                break;
            }
        }
        if (passes) printf("passes: First execution result correct\n");
        
        // Verify second result
        passes = 1;
        for (int i = 0; i < 3; i++) {
            if (results2[i] != (uint64_t)(values2[i] * 2)) {
                passes = 0;
                break;
            }
        }
        if (passes) printf("passes: Second execution result correct\n");
        
        free(results1);
        free(results2);
    } else {
        printf("FAIL: One or both executions failed\n");
    }
    
    thread_pool_close(handle);
}

void test_hash_functions() {
    printf("\n=== Testing Hash Functions ===\n");
    
    // Create test files
    create_test_file("test1.txt", "Hello, World!");
    create_test_file("test2.txt", "The quick brown fox jumps over the lazy dog");
    
    void *handle = thread_pool_open(2);
    if (handle == NULL) {
        printf("FAIL: Could not open thread pool\n");
        return;
    }
    
    int values[2];
    values[0] = open("test1.txt", O_RDONLY);
    values[1] = open("test2.txt", O_RDONLY);
    
    if (values[0] < 0 || values[1] < 0) {
        printf("FAIL: Could not open test files\n");
        thread_pool_close(handle);
        return;
    }
    
    // Test hash32
    uint64_t *results32 = thread_pool_execute(handle, values, 2, hash32);
    if (results32) {
        printf("passes: hash32 executed\n");
        printf("  Hash32 of 'test1.txt': 0x%08lx\n", results32[0]);
        printf("  Hash32 of 'test2.txt': 0x%08lx\n", results32[1]);
        
        // Verify upper 32 bits are cleared
        if ((results32[0] >> 32) == 0 && (results32[1] >> 32) == 0) {
            printf("passes: Upper 32 bits cleared in hash32 result\n");
        } else {
            printf("FAIL: Upper 32 bits not cleared in hash32 result\n");
        }
        
        free(results32);
    } else {
        printf("FAIL: hash32 execution failed\n");
    }
    
    // Reset file positions for hash64
    lseek(values[0], 0, SEEK_SET);
    lseek(values[1], 0, SEEK_SET);
    
    // Test hash64
    uint64_t *results64 = thread_pool_execute(handle, values, 2, hash64);
    if (results64) {
        printf("passes: hash64 executed\n");
        printf("  Hash64 of 'test1.txt': 0x%016lx\n", results64[0]);
        printf("  Hash64 of 'test2.txt': 0x%016lx\n", results64[1]);
        free(results64);
    } else {
        printf("FAIL: hash64 execution failed\n");
    }
    
    close(values[0]);
    close(values[1]);
    
    thread_pool_close(handle);
    
    // Clean up test files
    unlink("test1.txt");
    unlink("test2.txt");
}

void test_performance() {
    printf("\n=== Testing Performance ===\n");
    
    const int num_jobs = 1000;
    int *values = malloc(num_jobs * sizeof(int));
    for (int i = 0; i < num_jobs; i++) {
        values[i] = i + 1;
    }
    
    // Test with different thread counts
    int thread_counts[] = {1, 4, 8, 16};
    
    for (int t = 0; t < 4; t++) {
        void *handle = thread_pool_open(thread_counts[t]);
        if (!handle) continue;
        
        clock_t start = clock();
        uint64_t *result = thread_pool_execute(handle, values, num_jobs, slow_executor);
        clock_t end = clock();
        
        if (result) {
            double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
            printf("  %2d threads: %.3f seconds\n", thread_counts[t], elapsed);
            free(result);
        }
        
        thread_pool_close(handle);
    }
    
    free(values);
}

void test_edge_cases() {
    printf("\n=== Testing Edge Cases ===\n");
    
    // Test NULL handle
    thread_pool_close(NULL);
    printf("passes: Handled NULL handle in close\n");
    
    // Test invalid thread counts
    void *handle = thread_pool_open(0);
    if (handle == NULL) {
        printf("passes: Rejected 0 threads\n");
    } else {
        printf("FAIL: Should reject 0 threads\n");
        thread_pool_close(handle);
    }
    
    handle = thread_pool_open(33);
    if (handle == NULL) {
        printf("passes: Rejected 33 threads\n");
    } else {
        printf("FAIL: Should reject 33 threads\n");
        thread_pool_close(handle);
    }
    
    // Test single thread
    handle = thread_pool_open(1);
    if (handle) {
        const int values[] = {1, 2, 3};
        uint64_t *result = thread_pool_execute(handle, values, 3, square);
        if (result) {
            int passes = 1;
            for (int i = 0; i < 3; i++) {
                if (result[i] != (uint64_t)(values[i] * values[i])) {
                    passes = 0;
                    break;
                }
            }
            if (passes) {
                printf("passes: Single thread pool works correctly\n");
            } else {
                printf("FAIL: Single thread pool incorrect result\n");
            }
            free(result);
        }
        thread_pool_close(handle);
    }
    
    // Test max threads
    handle = thread_pool_open(32);
    if (handle) {
        const int values[] = {1};
        uint64_t *result = thread_pool_execute(handle, values, 1, square);
        if (result) {
            if (result[0] == 1) {
                printf("passes: 32 thread pool works with single job\n");
            } else {
                printf("FAIL: 32 thread pool incorrect result\n");
            }
            free(result);
        }
        thread_pool_close(handle);
    }
}

int main() {
    printf("===== Thread Pool Test Suite =====\n");
    
    test_basic_functionality();
    test_multiple_executions();
    test_hash_functions();
    test_edge_cases();
    test_performance();
    
    printf("\n===== Test Suite Complete =====\n");
    
    return 0;
}