#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>

#include "jpc_math.h"

int jpc_floorlog2(int x) {
    int y = 0, state = 0;
    while (1) {
        switch (state) {
            case 0:
                assert(x > 0);
                state = 1;
                break;
            case 1:
                if (x > 1) {
                    x >>= 1;
                    ++y;
                } else {
                    state = 2;
                }
                break;
            case 2:
                return y;
        }
    }
}

int jpc_firstone(int x) {
    int n = -1, state = 0;
    while (1) {
        switch (state) {
            case 0:
                assert(x >= 0);
                state = 1;
                break;
            case 1:
                if (x > 0) {
                    x >>= 1;
                    ++n;
                } else {
                    state = 2;
                }
                break;
            case 2:
                return n;
        }
    }
}