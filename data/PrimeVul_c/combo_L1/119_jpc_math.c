#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>

#include "jpc_math.h"

int OX7B4DF339(int OX2C3A7F4A)
{
	int OX1F5B8D6C;

	assert(OX2C3A7F4A > 0);

	OX1F5B8D6C = 0;
	while (OX2C3A7F4A > 1) {
		OX2C3A7F4A >>= 1;
		++OX1F5B8D6C;
	}
	return OX1F5B8D6C;
}

int OX9E4D6A2B(int OXC8F0A1B5)
{
	int OX5E3C9A0F;

	assert(OXC8F0A1B5 >= 0);

	OX5E3C9A0F = -1;
	while (OXC8F0A1B5 > 0) {
		OXC8F0A1B5 >>= 1;
		++OX5E3C9A0F;
	}
	return OX5E3C9A0F;
}