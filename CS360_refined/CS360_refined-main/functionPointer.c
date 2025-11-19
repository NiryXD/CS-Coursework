#include "fnptr.h"
#include <stdlib.h>

union FNPTR {
    ARITHPTR afunc;
    STRPTR   sfunc;
};

FNPTR *fnptr_new_arith(ARITHPTR func)
{
    if (!func) return NULL;
    FNPTR *ret = (FNPTR *)malloc(sizeof *ret);
    if (!ret) return NULL;
    ret->afunc = func;
    return ret;
}

FNPTR *fnptr_new_str(STRPTR func)
{
    if (!func) return NULL;
    FNPTR *ret = (FNPTR *)malloc(sizeof *ret);
    if (!ret) return NULL;
    ret->sfunc = func;
    return ret;
}

double fnptr_run_double(FNPTR *fp, double left, double right)
{
    if (!fp || !fp->afunc) return 0.0;
    return fp->afunc(left, right);
}

void fnptr_run_str(FNPTR *fp, char dst[], const char src[])
{
    if (!fp || !fp->sfunc) return;
    fp->sfunc(dst, src);
}

void fnptr_free(FNPTR *fp)
{
    free(fp);
}
