#include <stdio.h>

int main(int argc, char* argv[]) 
{
FILE *fl;
int a, b, c;

if (argv[1] == NULL) {
    printf ("not work lol");
    return -1;
}
fl = fopen(argv[1], "rb");
if (!fl) {
perror(argv[1]);
return -1;
}

while (fscanf(fl, "%d %d %d", &a, &b, &c) == 3 ) {
    printf ("%-4d * %-4d + %-4d = %d\n" ,a, b, c, a*b+c);
}
fclose(fl);

}
