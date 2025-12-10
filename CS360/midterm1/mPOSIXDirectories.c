/*
 * Program Description
 * -------------------
 * What this program does:
 *   Lists the entries inside a directory that you pass on the command line.
 *   For each entry, it prints the name and a human readable file-type tag.
 *
 * How it works in plain steps:
 *   1) Check that a directory path was provided.
 *   2) Open the directory using opendir.
 *   3) Repeatedly call readdir to get each entry (struct dirent).
 *   4) Skip the special entries "." and "..".
 *   5) Map d_type to a short text like DT_REG, DT_DIR, DT_LNK, etc.
 *   6) Print "name (TYPE)" with simple formatting.
 *   7) Close the directory with closedir.
 *
 * Notes:
 *   - d_type can be DT_UNKNOWN on some file systems. If you require exact types,
 *     call lstat on the entry path to determine the real type.
 *   - Always check that opendir succeeded before using the DIR*.
 */

#include <stdio.h>     // printf, perror
#include <dirent.h>    // DIR, struct dirent, opendir, readdir, closedir
#include <string.h>    // strcmp

int main(int argc, char *argv[])
{
    // Require a directory path
    if (argc < 2) {
        printf("Usage: %s <directory>\n", argv[0]);
        return 1;
    }

    DIR *dir = opendir(argv[1]); // try to open the directory
    if (!dir) {
        perror(argv[1]);         // print why it failed, e.g., "No such file or directory"
        return 2;
    }

    struct dirent *d;

    // Read entries until readdir returns NULL
    while ((d = readdir(dir)) != NULL) {
        // Skip the current and parent directory entries
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) {
            continue;
        }

        // Decide on a readable type string for the entry
        const char *type_str;
        switch (d->d_type) {
            case DT_REG:  type_str = "DT_REG";  break;  // regular file
            case DT_DIR:  type_str = "DT_DIR";  break;  // directory
            case DT_LNK:  type_str = "DT_LNK";  break;  // symbolic link
            case DT_FIFO: type_str = "DT_FIFO"; break;  // named pipe
            case DT_SOCK: type_str = "DT_SOCK"; break;  // socket
            case DT_CHR:  type_str = "DT_CHR";  break;  // char device
            case DT_BLK:  type_str = "DT_BLK";  break;  // block device
            case DT_UNKNOWN:
            default:
                type_str = "DT_UNKNOWN";                // unknown or not provided by FS
                break;
        }

        // %-10s means left justify name in a 10 character field
        printf("%-10s (%s)\n", d->d_name, type_str);
    }

    closedir(dir); // always close the directory
    return 0;
}
