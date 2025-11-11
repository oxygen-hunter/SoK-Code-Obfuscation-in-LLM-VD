pragma solidity ^0.4.10;

contract IntegerOverflowAdd {
    mapping (address => uint256) public balanceOf;

    function transfer(address _to, uint256 _value) public{
        require(balanceOf[msg.sender] >= _value);
        balanceOf[msg.sender] -= _value;
        balanceOf[_to] += _value;
    }
}

import ctypes

c_code = """
#include <stdint.h>

uint64_t add(uint64_t a, uint64_t b) {
    return a + b;
}
"""

with open("add.c", "w") as f:
    f.write(c_code)

import os
os.system("gcc -shared -o add.so -fPIC add.c")

add_lib = ctypes.CDLL("./add.so")
add_lib.add.argtypes = [ctypes.c_uint64, ctypes.c_uint64]
add_lib.add.restype = ctypes.c_uint64

def python_add(a, b):
    return add_lib.add(a, b)