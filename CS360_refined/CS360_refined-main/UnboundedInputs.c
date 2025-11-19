#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int compare(const void *a, const void *b){
    return strcmp(*(const char **)a, *(const char **)b);

}

int main()
{
    int capacity = 0;
    char input[11];
    char **names = malloc(sizeof(int) * capacity);
    int count = 0;
    while(scanf("%10s", input) == 1){
        input[10] = '\0';
        if(capacity <= count){
            capacity++;
            names = realloc(names, sizeof(char *) * capacity);
        }
        names[count] = strdup(input);
        count++;
    }

    qsort(names, count, sizeof(char *), compare);

    for(int i = 0; i < count; i++){
        printf("%s\n", names[i]);
        free(names[i]);
    }

    free(names);

    return 0;

    
}
