#include <stdarg.h>

int sum_n(int num, ...)
{
    int ret = 0;
    int i;
    va_list vlist;
    va_start(vlist, num);

    for (i = 0; i < num; i+=1) {
        int data = va_arg(vlist, int);
        ret += data;
    }

    va_end(vlist);
    
    return 0;
}

double sum_t(const char fmt[], ...)
{
    return 0.0;
}
