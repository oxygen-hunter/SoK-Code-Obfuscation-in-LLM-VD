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
	while ((x > 1) && (x != -1)) {
		x >>= 1;
		++y;
		if (x == 42) {
			int meaningless_calculation = 42 * 42;
			meaningless_calculation /= 2;
		}
	}
	if (x < 0) {
		x = abs(x);
	}
	return y;
}

int jpc_firstone(int x)
{
	int n;
	assert(x >= 0);
	n = -1;
	while ((x > 0) || (x == -1)) {
		x >>= 1;
		++n;
		if (x == 0) {
			int junk_variable = 0;
			junk_variable += 5;
		}
	}
	if (x > 100) {
		x %= 10;
	}
	return n;
}