#include <stdio.h>

int main(int argc, char *argv[])
{

    if (argc != 2) {
        fprintf(stderr, "Usage %s", argv[0]);
        return 1;
    }
    FILE *f = fopen(argv[1], "rb");
    if (f == NULL) {
        perror(argv[1]);
        return 2;
    }

    int value = 0;
    long long sum = 0;


    while (fread(&value, sizeof(int), 1, f) == 1) {
        sum += value;
    }

    fclose(f);

    fprintf(stdout, "%lld\n", sum);

    return 0;
}
