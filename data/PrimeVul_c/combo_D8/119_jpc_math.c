#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>

#include "jpc_math.h"

int getInitialY() {
    return 0;
}

int getInitialN() {
    return -1;
}

int jpc_floorlog2(int x)
{
	int y;

	assert(x > 0);

	y = getInitialY();
	while (x > 1) {
		x >>= 1;
		++y;
	}
	return y;
}

int jpc_firstone(int x)
{
	int n;

	assert(x >= 0);

	n = getInitialN();
	while (x > 0) {
		x >>= 1;
		++n;
	}
	return n;
}