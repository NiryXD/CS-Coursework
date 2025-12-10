// main.c  (no sprintf/snprintf)
#include <stdio.h>
#include <dirent.h>      // opendir, readdir, closedir
#include <sys/stat.h>    // stat, S_ISREG
#include <unistd.h>      // getcwd, chdir
#include <limits.h>      // PATH_MAX

#include "pagealloc.h"

static void list_regular_files(const char *path) {
    char oldcwd[PATH_MAX];

    // Save current working directory
    if (!getcwd(oldcwd, sizeof oldcwd)) {
        perror("getcwd");
        // continue anyway; if chdir fails, we'll just try listing "."
    }

    // Enter the directory we want to list
    if (chdir(path) == -1) {
        perror("chdir");
        return;
    }

    DIR *dir = opendir(".");
    if (!dir) {
        perror("opendir");
        // attempt to go back even on failure
        (void)chdir(oldcwd);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // skip dot-entries
        if (entry->d_name[0] == '.') continue;

        struct stat st;
        // stat by name; since we chdir'd, this is relative to 'path'
        if (stat(entry->d_name, &st) == 0 && S_ISREG(st.st_mode)) {
            // print the full-looking path without building a buffer
            printf("file: %s/%s\n", path, entry->d_name);
        }
    }
    closedir(dir);

    // go back to original directory
    if (chdir(oldcwd) == -1) {
        perror("chdir back");
    }
}

static void run_allocator_demo(void) {
    if (!page_init(12)) {
        puts("page_init failed");
        return;
    }
    printf("after init: taken=%zu free=%zu\n", pages_taken(), pages_free());

    void *a = page_alloc(3);
    void *b = page_alloc(2);
    printf("after a=3, b=2: taken=%zu free=%zu\n", pages_taken(), pages_free());

    page_free(b);
    printf("after free(b): taken=%zu free=%zu\n", pages_taken(), pages_free());

    void *c = page_alloc(10);
    printf("alloc(10) -> %s\n", c ? "ok" : "NULL");

    void *d = page_alloc(7);
    printf("after d=7: taken=%zu free=%zu\n", pages_taken(), pages_free());

    if (a) page_free((char *)a + 4096); // attempt to free middle of block (ignored)
    if (d) page_free(d);
    if (a) page_free(a);

    printf("after frees: taken=%zu free=%zu\n", pages_taken(), pages_free());
    page_deinit();
}

int main(void) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof cwd)) {
        printf("cwd: %s\n", cwd);
    } else {
        perror("getcwd");
    }

    // List regular files in the current directory (no sprintf/snprintf)
    list_regular_files(".");

    // Exercise your page allocator
    run_allocator_demo();

    puts("done");
    return 0;
}
