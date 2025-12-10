/*
 * du.c
 *
 * Simple directory tree builder and disk-usage calculator.
 *
 * This file provides functions to build an in-memory representation of a
 * directory (files and subdirectories), compute disk usage, and free the
 * structure when finished. The code below is the same logic as your
 * original, but with "layman" comments that explain what's happening and
 * why. I removed noisy or irrelevant comments and clarified tricky bits
 * like how paths are constructed, what stat() returns, and how recursion
 * works.
 *
 * Key concepts (short, plain English):
 * - stat(): asks the operating system for information about a path
 *   (is it a file or directory, how big is a file, permissions, etc.).
 * - opendir()/readdir()/closedir(): lets you open a directory and read
 *   the names (entries) inside it (files and subdirectories).
 * - recursion: to explore a directory tree we call the same function for
 *   each subdirectory (the function calls itself). We limit recursion
 *   depth to avoid walking too deep.
 * - strdup/malloc/free: basic memory management; strdup allocates space
 *   and copies a string. Every allocation must later be freed.
 *
 * Console (build/run) notes:
 *   gcc -Wall -Wextra -std=c11 -o du du.c
 *   (This produces an object you can link into a program that uses
 *    the functions declared in du.h.)
 */

#include "du.h"     /* header provided by the assignment */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

/*
 * Helper: create a List node representing a file or directory at `path`.
 * DepthLeft controls how deep we should descend into subdirectories.
 * Returns a newly-allocated node or NULL on error.
 */
struct List *MakeTree(const char *path, size_t DepthLeft) {
    struct stat PathStuff;
    if (stat(path, &PathStuff) != 0) {
        /* stat failed (path may not exist or is not accessible) */
        return NULL;
    }

    /* If it's a regular file, create a File node with its size. */
    if (S_ISREG(PathStuff.st_mode)) {
        struct List *fileNode = malloc(sizeof *fileNode);
        if (fileNode == NULL) return NULL;

        fileNode->name = strdup(path);
        if (fileNode->name == NULL) {
            free(fileNode);
            return NULL;
        }
        fileNode->type = File;
        fileNode->size = (size_t)PathStuff.st_size; /* file size in bytes */
        fileNode->next = NULL;
        return fileNode;
    }

    /* If it's a directory, build a Directory node and optionally
     * recurse into its entries (files and subdirectories). */
    if (S_ISDIR(PathStuff.st_mode)) {
        struct List *DirNode = malloc(sizeof *DirNode);
        if (DirNode == NULL) return NULL;

        DirNode->name = strdup(path);
        if (DirNode->name == NULL) {
            free(DirNode);
            return NULL;
        }
        DirNode->type = Directory;
        DirNode->contents = NULL;
        DirNode->next = NULL;

        /* If depth limit reached, stop here and return the directory node
         * without reading its children. This lets callers restrict how
         * deep the tree walk goes (useful for tests and preventing huge
         * traversals).
         */
        if (DepthLeft == 0) return DirNode;

        DIR *openDir = opendir(path);
        if (openDir == NULL) {
            /* Could not open directory (permissions, race, etc.). Return
             * the directory node without contents rather than failing.
             */
            return DirNode;
        }

        struct List *childHead = NULL; /* build a simple linked list of children */
        struct dirent *entry;

        while ((entry = readdir(openDir)) != NULL) {
            /* skip '.' and '..' entries that refer to current/parent dir */
            if (entry->d_name[0] == '.') {
                if (entry->d_name[1] == '\0') continue;
                if (entry->d_name[1] == '.' && entry->d_name[2] == '\0') continue;
            }

            /* Build the full path for the child: path + '/' (if needed)
             * + entry->d_name. Careful about trailing slashes. */
            size_t parentLength = strlen(path);
            size_t nameLength = strlen(entry->d_name);
            int needSlash = 0;
            if (parentLength > 0 && path[parentLength - 1] != '/') needSlash = 1;

            char *ChildPath = malloc(parentLength + needSlash + nameLength + 1);
            if (ChildPath == NULL) break; /* out of memory: stop adding children */

            if (parentLength > 0) memcpy(ChildPath, path, parentLength);
            if (needSlash) ChildPath[parentLength] = '/';
            if (nameLength > 0) memcpy(ChildPath + parentLength + needSlash, entry->d_name, nameLength);
            ChildPath[parentLength + needSlash + nameLength] = '\0';

            /* Ask the OS about the child so we know whether it is a file
             * or directory. If stat fails we skip the entry. */
            struct stat ChildInfo;
            int hasChild = (stat(ChildPath, &ChildInfo) == 0);
            struct List *childNode = NULL;

            if (hasChild) {
                if (S_ISDIR(ChildInfo.st_mode)) {
                    /* If we still have depth budget, recurse, otherwise just
                     * create an empty Directory node. */
                    if (DepthLeft > 0) {
                        childNode = MakeTree(ChildPath, DepthLeft - 1);
                        if (childNode == NULL) {
                            /* If recursion failed, fall back to creating a
                             * simple Directory node with no contents. */
                            childNode = malloc(sizeof *childNode);
                            if (childNode != NULL) {
                                childNode->name = strdup(ChildPath);
                                if (childNode->name == NULL) {
                                    free(childNode);
                                    childNode = NULL;
                                } else {
                                    childNode->type = Directory;
                                    childNode->contents = NULL;
                                    childNode->next = NULL;
                                }
                            }
                        }
                    } else {
                        childNode = malloc(sizeof *childNode);
                        if (childNode != NULL) {
                            childNode->name = strdup(ChildPath);
                            if (childNode->name == NULL) {
                                free(childNode);
                                childNode = NULL;
                            } else {
                                childNode->type = Directory;
                                childNode->contents = NULL;
                                childNode->next = NULL;
                            }
                        }
                    }
                } else if (S_ISREG(ChildInfo.st_mode)) {
                    /* Regular file: store its size */
                    childNode = malloc(sizeof *childNode);
                    if (childNode != NULL) {
                        childNode->name = strdup(ChildPath);
                        if (childNode->name == NULL) {
                            free(childNode);
                            childNode = NULL;
                        } else {
                            childNode->type = File;
                            childNode->size = (size_t)ChildInfo.st_size;
                            childNode->next = NULL;
                        }
                    }
                }
            }

            free(ChildPath);

            if (childNode != NULL) {
                /* push onto the head of the linked list of children */
                childNode->next = childHead;
                childHead = childNode;
            }
        }

        closedir(openDir);
        DirNode->contents = childHead;
        return DirNode;
    }

    /* Not a regular file or directory (e.g. symlink, socket): skip */
    return NULL;
}

/* Public functions required by du.h */
struct List *build_tree(const char starting_directory[], size_t max_depth) {
    if (starting_directory == NULL) return NULL;
    return MakeTree(starting_directory, max_depth);
}

/*
 * free_tree: recursively free all nodes allocated by build_tree.
 * We walk the linked lists and free names and nodes. For directories we
 * also free their contents recursively.
 */
void free_tree(struct List *tree){
    struct List *current = tree;
    while (current != NULL) {
        struct List *next = current->next;
        if (current->type == Directory && current->contents != NULL) {
            free_tree(current->contents);
        }
        free(current->name);
        free(current);
        current = next;
    }
}

/* Disk usage: sum sizes of files in the tree */
size_t disk_usage(const struct List *tree){
    size_t bytes = 0;
    const struct List *node = tree;
    while (node != NULL) {
        if (node->type == File) {
            bytes += node->size;
        } else if (node->type == Directory) {
            bytes += disk_usage(node->contents);
        }
        node = node->next;
    }
    return bytes;
}

/* file_size: returns the total size of files in the (possibly flat)
 * list pointed to by item. If item is a directory node, you can call
 * disk_usage(item->contents) instead; here we sum sizes across the
 * sibling list provided (this mirrors the original implementation).
 */
size_t file_size(const struct List *item){
    size_t bytes = 0;
    const struct List *p = item;
    while (p != NULL) {
        if (p->type == File) bytes += p->size;
        p = p->next;
    }
    return bytes;
}
