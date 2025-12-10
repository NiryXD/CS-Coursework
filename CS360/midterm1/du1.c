// du.c (clearer names version)
#include "du.h"

#include <dirent.h>     // opendir, readdir, closedir
#include <sys/stat.h>   // stat, S_ISDIR, S_ISREG
#include <unistd.h>     // POSIX types (optional)
#include <stdlib.h>     // malloc, free
#include <string.h>     // strlen, strdup, memcpy

/* Build a node for path. If it's a directory and depth_remaining > 0,
   populate its children recursively. */
static struct List *build_tree_internal(const char *path, size_t depth_remaining) {
    struct stat path_info;
    int stat_ok = stat(path, &path_info);
    if (stat_ok != 0) {
        return NULL;  // couldn't read metadata for this path
    }

    /* Handle regular file */
    if (S_ISREG(path_info.st_mode)) {
        struct List *file_node = (struct List *)malloc(sizeof *file_node);
        if (file_node == NULL) return NULL;

        file_node->name = strdup(path);
        if (file_node->name == NULL) {
            free(file_node);
            return NULL;
        }

        file_node->type = File;
        file_node->size = (size_t)path_info.st_size;
        file_node->next = NULL;
        return file_node;
    }

    /* Handle directory */
    if (S_ISDIR(path_info.st_mode)) {
        struct List *dir_node = (struct List *)malloc(sizeof *dir_node);
        if (dir_node == NULL) return NULL;

        dir_node->name = strdup(path);
        if (dir_node->name == NULL) {
            free(dir_node);
            return NULL;
        }
        dir_node->type = Directory;
        dir_node->contents = NULL;
        dir_node->next = NULL;

        /* Stop if we've reached the recursion depth limit */
        if (depth_remaining == 0) {
            return dir_node;
        }

        DIR *dir_stream = opendir(path);
        if (dir_stream == NULL) {
            /* Return an empty directory node if we can't open it */
            return dir_node;
        }

        struct List *children_head = NULL;
        struct dirent *entry;

        while ((entry = readdir(dir_stream)) != NULL) {
            /* Skip "." and ".." */
            if (entry->d_name[0] == '.') {
                if (entry->d_name[1] == '\0') continue;
                if (entry->d_name[1] == '.' && entry->d_name[2] == '\0') continue;
            }

            /* Build child_path = path + '/' + entry->d_name */
            size_t base_len = strlen(path);
            size_t name_len = strlen(entry->d_name);
            int need_slash = 1;
            if (base_len > 0) {
                if (path[base_len - 1] == '/') {
                    need_slash = 0;
                }
            }

            char *child_path = (char *)malloc(base_len + (size_t)need_slash + name_len + 1);
            if (child_path == NULL) {
                /* Allocation failed; stop scanning but keep what we have */
                break;
            }

            if (base_len > 0) {
                memcpy(child_path, path, base_len);
            }
            if (need_slash) {
                child_path[base_len] = '/';
                base_len += 1;
            }
            if (name_len > 0) {
                memcpy(child_path + base_len, entry->d_name, name_len);
            }
            child_path[base_len + name_len] = '\0';

            /* Stat the child to determine exact type and size */
            struct stat child_info;
            int child_stat_ok = stat(child_path, &child_info);
            struct List *child_node = NULL;

            if (child_stat_ok == 0) {
                if (S_ISDIR(child_info.st_mode)) {
                    if (depth_remaining > 0) {
                        child_node = build_tree_internal(child_path, depth_remaining - 1);
                        if (child_node == NULL) {
                            /* Fall back to an empty directory node */
                            child_node = (struct List *)malloc(sizeof *child_node);
                            if (child_node != NULL) {
                                child_node->name = strdup(child_path);
                                if (child_node->name == NULL) {
                                    free(child_node);
                                    child_node = NULL;
                                } else {
                                    child_node->type = Directory;
                                    child_node->contents = NULL;
                                    child_node->next = NULL;
                                }
                            }
                        }
                    } else {
                        /* Depth limit reached: create empty directory node */
                        child_node = (struct List *)malloc(sizeof *child_node);
                        if (child_node != NULL) {
                            child_node->name = strdup(child_path);
                            if (child_node->name == NULL) {
                                free(child_node);
                                child_node = NULL;
                            } else {
                                child_node->type = Directory;
                                child_node->contents = NULL;
                                child_node->next = NULL;
                            }
                        }
                    }
                } else if (S_ISREG(child_info.st_mode)) {
                    child_node = (struct List *)malloc(sizeof *child_node);
                    if (child_node != NULL) {
                        child_node->name = strdup(child_path);
                        if (child_node->name == NULL) {
                            free(child_node);
                            child_node = NULL;
                        } else {
                            child_node->type = File;
                            child_node->size = (size_t)child_info.st_size;
                            child_node->next = NULL;
                        }
                    }
                } else {
                    /* Ignore other special types (symlinks, sockets, etc.) */
                }
            }

            free(child_path);

            if (child_node != NULL) {
                /* Prepend to the list of children */
                child_node->next = children_head;
                children_head = child_node;
            }
        }

        closedir(dir_stream);
        dir_node->contents = children_head;
        return dir_node;
    }

    /* Ignore non-regular, non-directory types */
    return NULL;
}

/* -------- required functions -------- */

struct List *build_tree(const char starting_directory[], size_t max_depth) {
    if (starting_directory == NULL) return NULL;
    return build_tree_internal(starting_directory, max_depth);
}

void free_tree(struct List *tree) {
    struct List *current = tree;
    while (current != NULL) {
        struct List *next_sibling = current->next;

        if (current->type == Directory) {
            if (current->contents != NULL) {
                free_tree(current->contents);
            }
        }

        free(current->name);
        free(current);

        current = next_sibling;
    }
}

size_t disk_usage(const struct List *tree) {
    size_t total_bytes = 0;
    const struct List *current = tree;

    while (current != NULL) {
        if (current->type == File) {
            total_bytes += current->size;
        } else if (current->type == Directory) {
            total_bytes += disk_usage(current->contents);
        }
        current = current->next;
    }

    return total_bytes;
}

size_t file_size(const struct List *item) {
    size_t total_bytes = 0;
    const struct List *child;

    if (item == NULL) return 0;

    if (item->type == File) {
        return item->size;
    }

    child = item->contents;
    while (child != NULL) {
        if (child->type == File) {
            total_bytes += child->size;
        }
        child = child->next;
    }

    return total_bytes;
}
