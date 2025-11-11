# Import necessary libraries
import ctypes
import os

# Define paths for the C dynamic library
lib_path = os.path.abspath('jpc_math_lib.so')

# Load the C dynamic library
jpc_math_lib = ctypes.CDLL(lib_path)

# Define the argument and return types for the functions
jpc_math_lib.jpc_floorlog2.argtypes = [ctypes.c_int]
jpc_math_lib.jpc_floorlog2.restype = ctypes.c_int
jpc_math_lib.jpc_firstone.argtypes = [ctypes.c_int]
jpc_math_lib.jpc_firstone.restype = ctypes.c_int

# Define Python functions that call the C functions
def jpc_floorlog2(x):
    assert x > 0
    return jpc_math_lib.jpc_floorlog2(x)

def jpc_firstone(x):
    assert x >= 0
    return jpc_math_lib.jpc_firstone(x)

# Usage
if __name__ == "__main__":
    print(jpc_floorlog2(16))
    print(jpc_firstone(10))
```

```c
#include <assert.h>
#include <stdio.h>

/* Calculate the integer quantity floor(log2(x)), where x is a positive
  integer. */
int jpc_floorlog2(int x)
{
	int y;

	/* The argument must be positive. */
	assert(x > 0);

	y = 0;
	while (x > 1) {
		x >>= 1;
		++y;
	}
	return y;
}

/* Calculate the bit position of the first leading one in a nonnegative
  integer. */
/* This function is the basically the same as ceillog2(x), except that the
  allowable range for x is slightly different. */
int jpc_firstone(int x)
{
	int n;

	/* The argument must be nonnegative. */
	assert(x >= 0);

	n = -1;
	while (x > 0) {
		x >>= 1;
		++n;
	}
	return n;
}