#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>

#include "jpc_math.h"

int jpc_floorlog2(int x)
{
    int y;
    asm (
        "test %[x], %[x]\n\t"
        "jle 1f\n\t"
        "mov $0, %[y]\n\t"
        "2:\n\t"
        "shr $1, %[x]\n\t"
        "add $1, %[y]\n\t"
        "cmp $1, %[x]\n\t"
        "jg 2b\n\t"
        "jmp 3f\n\t"
        "1:\n\t"
        "int $3\n\t"
        "3:\n\t"
        : [y] "=r" (y)
        : [x] "r" (x)
        : "cc"
    );
    return y;
}

int jpc_firstone(int x)
{
    int n;
    asm (
        "test %[x], %[x]\n\t"
        "jl 1f\n\t"
        "mov $-1, %[n]\n\t"
        "2:\n\t"
        "shr $1, %[x]\n\t"
        "add $1, %[n]\n\t"
        "test %[x], %[x]\n\t"
        "jg 2b\n\t"
        "jmp 3f\n\t"
        "1:\n\t"
        "int $3\n\t"
        "3:\n\t"
        : [n] "=r" (n)
        : [x] "r" (x)
        : "cc"
    );
    return n;
}