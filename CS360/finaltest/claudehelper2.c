// Example main.c showing how to use exam_helpers for clean, readable code
// Compile with: gcc -std=gnu11 -Wall main.c exam_helpers.c -lpthread

#include "exam_helpers.h"

// ============================================================================
// EXAMPLE 1: Your midterm rewritten with helpers
// ============================================================================

/*
 * Original midterm main() rewritten to be clean and simple
 */
void* my_worker(void *arg) {
    Work *w = (Work*)arg;
    w->result = w->param[0] + w->param[1] + w->param[2];  // Example work
    return NULL;
}

void my_report(Work *w) {
    write_line("Result: %d + %d + %d = %d", 
               w->param[0], w->param[1], w->param[2], w->result);
}

int example_midterm_style(int argc, char *argv[]) {
    // Parse command line args
    CmdArgs *args = parse_args(argc, argv);
    if (get_arg_count(args) < 2) {
        write_line("Usage: %s <input_file>", get_arg(args, 0));
        free_args(args);
        return 1;
    }
    
    // Read work from stdin
    Work *work;
    int capacity;
    int work_count = read_work_stdin(&work, &capacity);
    
    // Create threads
    ThreadArray *threads = thread_array_create(work_count);
    
    // Launch all threads
    for (int i = 0; i < work_count; i++) {
        pthread_t tid;
        pthread_create(&tid, NULL, my_worker, &work[i]);
        thread_array_add(threads, tid);
    }
    
    // Join and report
    thread_array_join_all(threads);
    for (int i = 0; i < work_count; i++) {
        my_report(&work[i]);
    }
    
    // Cleanup
    thread_array_free(threads);
    free(work);
    free_args(args);
    return 0;
}

// ============================================================================
// EXAMPLE 2: Thread Pool Version (more advanced)
// ============================================================================

void* pool_worker(void *arg) {
    Work *w = (Work*)arg;
    w->result = w->param[0] * w->param[1] * w->param[2];
    return NULL;
}

int example_with_threadpool(void) {
    // Create thread pool with 4 workers
    ThreadPool *pool = threadpool_create(4);
    
    // Read work
    Work *work;
    int capacity;
    int work_count = read_work_stdin(&work, &capacity);
    
    // Push all work to pool (marshaller pattern)
    for (int i = 0; i < work_count; i++) {
        threadpool_push(pool, pool_worker, &work[i]);
    }
    
    // Wait for completion
    threadpool_wait(pool);
    
    // Report results
    for (int i = 0; i < work_count; i++) {
        write_line("Work %d result: %d", i, work[i].result);
    }
    
    // Cleanup
    threadpool_destroy(pool);
    free(work);
    return 0;
}

// ============================================================================
// EXAMPLE 3: File I/O with Buffered Streams
// ============================================================================

int example_file_io(const char *input_file, const char *output_file) {
    // Open input file for reading
    BufferedFile *in = open_file_buffered(input_file, "r");
    if (!in) {
        write_line("Error: Cannot open %s", input_file);
        return 1;
    }
    
    // Open output file for writing
    BufferedFile *out = open_file_buffered(output_file, "w");
    if (!out) {
        close_file_buffered(in);
        write_line("Error: Cannot open %s", output_file);
        return 1;
    }
    
    // Read all lines
    char **lines;
    int count;
    read_all_lines(in, &lines, &count);
    
    // Process and write
    for (int i = 0; i < count; i++) {
        char *processed = str_concat("Processed: ", lines[i]);
        write_line_buffered(out, processed);
        free(processed);
    }
    
    // Cleanup
    free_lines(lines, count);
    close_file_buffered(in);
    close_file_buffered(out);
    return 0;
}

// ============================================================================
// EXAMPLE 4: String Manipulation
// ============================================================================

void example_string_ops(void) {
    // Copy
    char *str = str_copy("Hello, World!");
    write_line("Original: %s", str);
    
    // Find substring
    char *found = str_find(str, "World");
    if (found) {
        write_line("Found 'World' at position: %ld", found - str);
    }
    
    // Get substring
    char *sub = str_substring(str, 0, 5);
    write_line("Substring: %s", sub);  // "Hello"
    
    // Replace
    char *replaced = str_replace(str, "World", "C Programming");
    write_line("Replaced: %s", replaced);
    
    // Concat
    char *concat = str_concat(sub, " there!");
    write_line("Concatenated: %s", concat);
    
    // Split
    int count;
    char **parts = str_split("one,two,three,four", ",", &count);
    write_line("Split into %d parts:", count);
    for (int i = 0; i < count; i++) {
        write_line("  [%d] %s", i, parts[i]);
    }
    
    // Cleanup
    free(str);
    free(sub);
    free(replaced);
    free(concat);
    free_split(parts, count);
}

// ============================================================================
// EXAMPLE 5: Signal Handling with Graceful Shutdown
// ============================================================================

void* long_running_worker(void *arg) {
    int *id = (int*)arg;
    
    while (!should_shutdown()) {
        write_line("Worker %d is running...", *id);
        sleep(1);
    }
    
    write_line("Worker %d shutting down gracefully", *id);
    return NULL;
}

int example_signal_handling(void) {
    // Setup signal handlers
    setup_graceful_shutdown();
    
    // Create worker threads
    ThreadArray *threads = thread_array_create(2);
    int ids[] = {1, 2};
    
    for (int i = 0; i < 2; i++) {
        pthread_t tid;
        pthread_create(&tid, NULL, long_running_worker, &ids[i]);
        thread_array_add(threads, tid);
    }
    
    write_line("Press Ctrl+C to gracefully shutdown...");
    
    // Wait for shutdown signal
    while (!should_shutdown()) {
        sleep(1);
    }
    
    write_line("Shutdown signal received, waiting for workers...");
    thread_array_join_all(threads);
    
    thread_array_free(threads);
    return 0;
}

// ============================================================================
// EXAMPLE 6: Time Functions
// ============================================================================

void example_time_functions(void) {
    // Get current time
    TimeInfo *now = get_current_time();
    write_line("Current time: %s", now->formatted);
    
    // Custom format
    char *custom = format_time(now->raw_time, "%A, %B %d, %Y");
    write_line("Custom format: %s", custom);
    
    // Get simple timestamp
    char *ts = get_timestamp();
    write_line("Timestamp: %s", ts);
    
    // Parse a time string
    time_t parsed = parse_time("2024-12-25 10:30:00", "%Y-%m-%d %H:%M:%S");
    char *parsed_str = format_time(parsed, "%Y-%m-%d %H:%M:%S");
    write_line("Parsed time: %s", parsed_str);
    
    // Calculate elapsed time
    time_t start = time(NULL);
    sleep(2);
    time_t end = time(NULL);
    write_line("Elapsed: %.0f seconds", elapsed_seconds(start, end));
    
    // Cleanup
    free_time_info(now);
    free(custom);
    free(ts);
    free(parsed_str);
}

// ============================================================================
// EXAMPLE 7: Dynamic Array
// ============================================================================

void example_dynamic_array(void) {
    // Create dynamic array for integers
    DynamicArray *arr = darray_create(sizeof(int), 4);
    
    // Push elements (auto-resizes)
    for (int i = 0; i < 20; i++) {
        int val = i * 10;
        darray_push(arr, &val);
    }
    
    // Access elements
    write_line("Array has %d elements:", arr->count);
    for (int i = 0; i < arr->count; i++) {
        int *val = (int*)darray_get(arr, i);
        write_line("  [%d] = %d", i, *val);
    }
    
    darray_free(arr);
}

// ============================================================================
// MAIN - Uncomment the example you want to run
// ============================================================================

int main(int argc, char *argv[]) {
    // Uncomment ONE of these to test:
    
    // return example_midterm_style(argc, argv);
    // return example_with_threadpool();
    // return example_file_io("input.txt", "output.txt");
    example_string_ops(); return 0;
    // return example_signal_handling();
    // example_time_functions(); return 0;
    // example_dynamic_array(); return 0;
    
    (void)argc; (void)argv;  // Suppress warnings
    return 0;
}