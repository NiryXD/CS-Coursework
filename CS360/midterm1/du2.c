// du.c (layman/clear names)
#include "du.h"

#include <dirent.h>     // opendir, readdir, closedir
#include <sys/stat.h>   // stat, S_ISDIR, S_ISREG
#include <unistd.h>     // POSIX types (optional)
#include <stdlib.h>     // malloc, free
#include <string.h>     // strlen, strdup, memcpy

/* Build a node for "start_path". If it's a folder and depth_left > 0,
   also list its children (files/folders) by recursing. */
static struct List *build_tree_recursive(const char *start_path, size_t depth_left) {
    struct stat file_info;
    int stat_ok = stat(start_path, &file_info);
    if (stat_ok != 0) {
        return NULL;  // can't read info about this path
    }

    /* Handle a regular file */
    if (S_ISREG(file_info.st_mode)) {
        struct List *file_node = (struct List *)malloc(sizeof *file_node);
        if (file_node == NULL) return NULL;

        file_node->name = strdup(start_path);
        if (file_node->name == NULL) {
            free(file_node);
            return NULL;
        }

        file_node->type = File;
        file_node->size = (size_t)file_info.st_size;  // bytes
        file_node->next = NULL;
        return file_node;
    }

    /* Handle a directory (folder) */
    if (S_ISDIR(file_info.st_mode)) {
        struct List *folder_node = (struct List *)malloc(sizeof *folder_node);
        if (folder_node == NULL) return NULL;

        folder_node->name = strdup(start_path);
        if (folder_node->name == NULL) {
            free(folder_node);
            return NULL;
        }
        folder_node->type = Directory;
        folder_node->contents = NULL;  // will point to first child later
        folder_node->next = NULL;      // sibling in parent's list (set elsewhere)

        /* Stop if we hit the depth limit */
        if (depth_left == 0) {
            return folder_node;
        }

        DIR *open_directory = opendir(start_path);
        if (open_directory == NULL) {
            /* Return an empty folder node if we can't open it */
            return folder_node;
        }

        struct List *first_child = NULL;      // head of this folder's child list
        struct dirent *directory_entry;

        while ((directory_entry = readdir(open_directory)) != NULL) {
            /* Skip "." and ".." */
            if (directory_entry->d_name[0] == '.') {
                if (directory_entry->d_name[1] == '\0') continue;
                if (directory_entry->d_name[1] == '.' && directory_entry->d_name[2] == '\0') continue;
            }

            /* Build full child path: parent + "/" + entry_name */
            size_t parent_len = strlen(start_path);
            size_t entry_len = strlen(directory_entry->d_name);
            int needs_separator = 1;
            if (parent_len > 0) {
                if (start_path[parent_len - 1] == '/') {
                    needs_separator = 0;
                }
            }

            char *child_full_path = (char *)malloc(parent_len + (size_t)needs_separator + entry_len + 1);
            if (child_full_path == NULL) {
                /* Out of memory: stop scanning, keep what we have */
                break;
            }

            if (parent_len > 0) {
                memcpy(child_full_path, start_path, parent_len);
            }
            if (needs_separator) {
                child_full_path[parent_len] = '/';
                parent_len += 1;
            }
            if (entry_len > 0) {
                memcpy(child_full_path + parent_len, directory_entry->d_name, entry_len);
            }
            child_full_path[parent_len + entry_len] = '\0';

            /* Get info about the child to know if it's a file or folder */
            struct stat child_info;
            int child_stat_ok = stat(child_full_path, &child_info);
            struct List *child_node = NULL;

            if (child_stat_ok == 0) {
                if (S_ISDIR(child_info.st_mode)) {
                    /* Recurse into subfolder if depth allows; otherwise record an empty folder node */
                    if (depth_left > 0) {
                        child_node = build_tree_recursive(child_full_path, depth_left - 1);
                        if (child_node == NULL) {
                            /* Fallback: make an empty folder node so we still list it */
                            child_node = (struct List *)malloc(sizeof *child_node);
                            if (child_node != NULL) {
                                child_node->name = strdup(child_full_path);
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
                        child_node = (struct List *)malloc(sizeof *child_node);
                        if (child_node != NULL) {
                            child_node->name = strdup(child_full_path);
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
                        child_node->name = strdup(child_full_path);
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
                    /* Ignore special types (symlink, socket, device, etc.) */
                }
            }

            free(child_full_path);

            if (child_node != NULL) {
                /* Insert at front of child list */
                child_node->next = first_child;
                first_child = child_node;
            }
        }

        closedir(open_directory);
        folder_node->contents = first_child;
        return folder_node;
    }

    /* Ignore types that are neither regular files nor directories */
    return NULL;
}

/* ===== Required functions from du.h ===== */

struct List *build_tree(const char starting_directory[], size_t max_depth) {
    if (starting_directory == NULL) return NULL;
    return build_tree_recursive(starting_directory, max_depth);
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
    const struct List *node = tree;

    while (node != NULL) {
        if (node->type == File) {
            total_bytes += node->size;
        } else if (node->type == Directory) {
            total_bytes += disk_usage(node->contents);
        }
        node = node->next;
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
