#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>

#include "jpc_math.h"

int jpc_floorlog2_recursive(int x, int y) {
    return (x > 1) ? jpc_floorlog2_recursive(x >> 1, y + 1) : y;
}

int jpc_floorlog2(int x) {
    assert(x > 0);
    return jpc_floorlog2_recursive(x, 0);
}

int jpc_firstone_recursive(int x, int n) {
    return (x > 0) ? jpc_firstone_recursive(x >> 1, n + 1) : n;
}

int jpc_firstone(int x) {
    assert(x >= 0);
    return jpc_firstone_recursive(x, -1);
}