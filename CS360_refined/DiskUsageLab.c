// du.h has been written for you, and it's read only.
#include "du.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

// helper, builds node
struct List *MakeTree(const char *path, size_t DepthLeft) {
    struct stat PathStuff;
    int statYes = stat(path, &PathStuff);
    if (statYes != 0) {
        return NULL;
    }

    // https://pubs.opengroup.org/onlinepubs/7908799/xsh/sysstat.h.html
    if (S_ISREG(PathStuff.st_mode)) {
    // I was confused what sys/stat.h did so I had Chat explain it to me
    // I even uploaded the lecture notes and gave it online references to get a better understanding
        struct List *fileNode = (struct List *)malloc(sizeof *fileNode);
        if (fileNode == NULL)
        return NULL;
    // Senario for regular file
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
// Senario for directory
    if (S_ISDIR(PathStuff.st_mode)) {
        struct List *DirNode = (struct List *)malloc(sizeof *DirNode);
        if (DirNode == NULL)
        return NULL;

        DirNode->name = strdup(path);
        if (DirNode->name == NULL) {
            free(DirNode);
            return NULL;
        
        
        }
        DirNode->type = Directory;
        DirNode->contents = NULL;
        DirNode->next = NULL;

        if (DepthLeft == 0){
            return DirNode;
        }

        DIR *openDir = opendir(path);
        if (openDir == NULL) {
            return DirNode;
        }

        struct List *childHead = NULL;
        struct dirent *enter;

        while ((enter = readdir(openDir)) != NULL) {
            if (enter->d_name[0] == '.') {
                if (enter->d_name[1] == '\0')
                continue;
                if (enter->d_name[1] == '.' && enter->d_name[2] == '\0')
                continue;

            }
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
                break;
            }

            if (parentLength > 0){
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

            struct stat ChildINfo;
            int hasChild = (stat(ChildPath, &ChildINfo) == 0);
            struct List *childNode = NULL;

            if(hasChild) {
                if(S_ISDIR(ChildINfo.st_mode)) {
                    if (DepthLeft > 0) {
                        childNode = MakeTree(ChildPath, DepthLeft - 1);
                        if (childNode == NULL) {
                            childNode = (struct List *)malloc(sizeof *childNode);
                            if (childNode != NULL) {
                                childNode->name = strdup(ChildPath);
                                if (childNode->name == NULL) {
                                    free(childNode);
                                    childNode = NULL;
                                }
                                else {
                                    childNode->type = Directory;
        childNode->contents = NULL;
        childNode->next = NULL;
                                }
                            }
                        }
                    }
                    else {
                        childNode = (struct List *)malloc(sizeof *childNode);
                        if (childNode != NULL) {
                            childNode->name = strdup(ChildPath);
                            if (childNode->name == NULL) {
                                free(childNode);
                                childNode = NULL;
                            }
                            else {
                                childNode->type = Directory;
        childNode->contents = NULL;
        childNode->next = NULL;
                            }
                        }
                    }
                }
                else if (S_ISREG(ChildINfo.st_mode)) {
                    childNode = (struct List *)malloc(sizeof *childNode);
                    if (childNode != NULL) {
                        childNode->name = strdup(ChildPath);
                        if (childNode->name == NULL) {
                            free(childNode);
                            childNode = NULL;
                        }
                        else {
                            childNode->type = File;
        childNode->size = (size_t)ChildINfo.st_size;
        childNode->next = NULL;
                        }
                    }
                }
            }
free(ChildPath);

if (childNode != NULL) {
    childNode->next = childHead;
    // linked list fer childnren
    childHead = childNode;
}
        }

        closedir(openDir);
        DirNode->contents = childHead;
        return DirNode;

    }

    return NULL;

}


// these are the prototypes given to you in du.h. You need
// to write their functionality here.

struct List *build_tree(const char starting_directory[], size_t max_depth) {
    if(starting_directory == NULL) {
        return NULL;
    }
    return MakeTree(starting_directory, max_depth);
}
void free_tree(struct List *tree){
    // recursively frees
    struct List *current = tree;
    while (current != NULL) {
        struct List *nextkid = current->next; // remember before freeing

        if (current->type == Directory) {
            if (current->contents != NULL) {
                free_tree(current->contents);
            }
        }

        free(current->name);
        free(current);

        current = nextkid;
    }
}

size_t disk_usage(const struct List *tree){
    size_t bites = 0;
    const struct List *node = tree;
    while (node != NULL) {
        if (node->type == File) {
            bites += node->size;
        }
        else if (node->type == Directory) {
            bites += disk_usage(node->contents);
        }
        node = node->next;
    }
    return bites;
}
size_t file_size(const struct List *item){
    size_t bites = 0;
   /* const struct List *child;

    if (item == NULL)
    return 0;

    if (item->type == File) {
        return 0;
    }

    child = item->contents;
    while (child != NULL) {
        if ( child->type == File) {
            bites += child->size;
        }
        child = child->next; */

        const struct List *p = item;
        while(p != NULL) {
            if (p->type == File) {
                bites += p->size;
            }
            p = p->next;
        }
        return bites;
    }



