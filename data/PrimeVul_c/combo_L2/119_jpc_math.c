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
	assert(x > 0);
	y = 0;
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
	n = -1;
	while (x > 0) {
		x >>= 1;
		++n;
	}
	return n;
}