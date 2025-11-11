#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>

#include "jpc_math.h"

/* Calculate the integer quantity floor(log2(x)), where x is a positive
  integer. */
int jpc_floorlog2(int x)
{
	int arr[2];

	/* The argument must be positive. */
	assert(x > 0);

	arr[1] = 0;
	while (x > 1) {
		x >>= 1;
		++arr[1];
	}
	return arr[1];
}

/* Calculate the bit position of the first leading one in a nonnegative
  integer. */
/* This function is the basically the same as ceillog2(x), except that the
  allowable range for x is slightly different. */
int jpc_firstone(int x)
{
	int a, b[1];

	/* The argument must be nonnegative. */
	assert(x >= 0);

	a = -1;
	b[0] = a;
	while (x > 0) {
		x >>= 1;
		++b[0];
	}
	return b[0];
}