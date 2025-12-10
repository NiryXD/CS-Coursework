// du.c
// du.h has been written for you, and it's read only.
#include "du.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

/* helper */
static struct List *MakeTree(const char *path, size_t DepthLeft) {
    struct stat PathStuff;
    int statYes = stat(path, &PathStuff);
    if (statYes != 0) {
        return NULL;
    }

    /* Regular file */
    if (S_ISREG(PathStuff.st_mode)) {
        struct List *fileNode = (struct List *)malloc(sizeof *fileNode);
        if (fileNode == NULL) {
            return NULL;
        }

        fileNode->name = strdup(path);
        if (fileNode->name == NULL) {
            free(fileNode);
            return NULL;
        }

        fileNode->type = File;
        fileNode->size = (size_t)PathStuff.st_size;
        fileNode->next = NULL;
        return fileNode;
    }

    /* Directory */
    if (S_ISDIR(PathStuff.st_mode)) {
        struct List *DirNode = (struct List *)malloc(sizeof *DirNode);
        if (DirNode == NULL) {
            return NULL;
        }

        DirNode->name = strdup(path);
        if (DirNode->name == NULL) {
            free(DirNode);
            return NULL;
        }

        DirNode->type = Directory;
        DirNode->contents = NULL;
        DirNode->next = NULL;

        /* depth limit */
        if (DepthLeft == 0) {
            return DirNode;
        }

        DIR *openDir = opendir(path);
        if (openDir == NULL) {
            return DirNode; /* return empty directory node if cannot open */
        }

        struct List *childHead = NULL;
        struct dirent *enter;

        while ((enter = readdir(openDir)) != NULL) {
            /* skip "." and ".." */
            if (enter->d_name[0] == '.') {
                if (enter->d_name[1] == '\0') {
                    continue;
                }
                if (enter->d_name[1] == '.' && enter->d_name[2] == '\0') {
                    continue;
                }
            }

            /* build ChildPath = path + "/" + d_name (avoid double slashes) */
            size_t parentLength = strlen(path);
            size_t enterLength = strlen(enter->d_name);
            int AddOn = 1;
            if (parentLength > 0) {
                if (path[parentLength - 1] == '/') {
                    AddOn = 0;
                }
            }

            char *ChildPath = (char *)malloc(parentLength + (size_t)AddOn + enterLength + 1);
            if (ChildPath == NULL) {
                /* out of memory; stop scanning more entries */
                break;
            }

            if (parentLength > 0) {
                memcpy(ChildPath, path, parentLength);
            }
            if (AddOn) {
                ChildPath[parentLength] = '/';
                parentLength += 1;
            }
            if (enterLength > 0) {
                memcpy(ChildPath + parentLength, enter->d_name, enterLength);
            }
            ChildPath[parentLength + enterLength] = '\0';

            /* stat child to know type/size (d_type may be unknown) */
            struct stat childInfo;
            int haveChild = (stat(ChildPath, &childInfo) == 0);
            struct List *childNode = NULL;

            if (haveChild) {
                if (S_ISDIR(childInfo.st_mode)) {
                    /* recurse if depth allows; else record empty directory node */
                    if (DepthLeft > 0) {
                        childNode = MakeTree(ChildPath, DepthLeft - 1);
                        if (childNode == NULL) {
                            childNode = (struct List *)malloc(sizeof *childNode);
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
                        childNode = (struct List *)malloc(sizeof *childNode);
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
                } else if (S_ISREG(childInfo.st_mode)) {
                    childNode = (struct List *)malloc(sizeof *childNode);
                    if (childNode != NULL) {
                        childNode->name = strdup(ChildPath);
                        if (childNode->name == NULL) {
                            free(childNode);
                            childNode = NULL;
                        } else {
                            childNode->type = File;
                            childNode->size = (size_t)childInfo.st_size;
                            childNode->next = NULL;
                        }
                    }
                } else {
                    /* ignore other types per assignment */
                }
            }

            free(ChildPath);

            if (childNode != NULL) {
                /* prepend into child list */
                childNode->next = childHead;
                childHead = childNode;
            }
        }

        closedir(openDir);
        DirNode->contents = childHead;
        return DirNode;
    }

    /* not regular file or directory: ignore */
    return NULL;
}

/* ---------------- required API from du.h ---------------- */

struct List *build_tree(const char starting_directory[], size_t max_depth) {
    if (starting_directory == NULL) {
        return NULL;
    }
    return MakeTree(starting_directory, max_depth);
}

void free_tree(struct List *tree) {
    struct List *p = tree;
    while (p != NULL) {
        struct List *next = p->next;

        if (p->type == Directory) {
            if (p->contents != NULL) {
                free_tree(p->contents);
            }
        }

        free(p->name);
        free(p);

        p = next;
    }
}

size_t disk_usage(const struct List *tree) {
    size_t total = 0;
    const struct List *p = tree;

    while (p != NULL) {
        if (p->type == File) {
            total += p->size;
        } else if (p->type == Directory) {
            total += disk_usage(p->contents);
        }
        p = p->next;
    }

    return total;
}

size_t file_size(const struct List *item) {
    size_t total = 0;
    const struct List *p;

    if (item == NULL) {
        return 0;
    }

    if (item->type == File) {
        return item->size;
    }

    p = item->contents;
    while (p != NULL) {
        if (p->type == File) {
            total += p->size;
        }
        p = p->next;
    }

    return total;
}
