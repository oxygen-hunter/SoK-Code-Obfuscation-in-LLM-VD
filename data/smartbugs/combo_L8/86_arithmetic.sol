pragma solidity ^0.4.23;

contract IntegerOverflowSingleTransaction {
    uint public c = 1;

    function overflowaddtostate(uint256 i) public {
        c += i;
    }
    
    function overflowmultostate(uint256 i) public {
        c *= i;
    }

    function underflowtostate(uint256 i) public {
        c -= i;
    }
    
    function overflowlocalonly(uint256 i) public {
        uint r = c + i;
    }
    
    function overflowmulocalonly(uint256 i) public {
        uint r = c * i;
    }

    function underflowlocalonly(uint256 i) public {
       	uint r = c - i;
    }
}
```
```python
import ctypes

# Load C library
clib = ctypes.CDLL('./liboverflow.dll')

def call_clib_func(func_name, *args):
    func = getattr(clib, func_name)
    func.argtypes = [ctypes.c_uint] * len(args)
    func.restype = ctypes.c_uint
    return func(*args)

# Example usage
result_add = call_clib_func('overflowaddtostate', 10)
result_mul = call_clib_func('overflowmultostate', 5)
result_sub = call_clib_func('underflowtostate', 3)

print(result_add, result_mul, result_sub)