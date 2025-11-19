#include <stdio.h>
#include <dirent.h>
#include <string.h>

int main(int argc, char *argv[])
{
      if (argc < 2) {
        printf("Usage: %s <directory>\n", argv[0]);
        return 1;
    }
    
    DIR *dir;
    struct dirent *d;

    dir = opendir(argv[1]);
    
    while (NULL != (d = readdir(dir))) {
      
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) {
            continue;
        }

        // Determine the file type string
        const char *type_str;
        switch (d->d_type) {
            case DT_REG:
                type_str = "DT_REG";
                break;
            case DT_DIR:
                type_str = "DT_DIR";
                break;
            case DT_LNK:
                type_str = "DT_LNK";
                break;
            case DT_FIFO:
                type_str = "DT_FIFO";
                break;
        }

    
        printf("%-10s (%s)\n", d->d_name, type_str);
    }
    
    closedir(dir);
    return 0;
}
