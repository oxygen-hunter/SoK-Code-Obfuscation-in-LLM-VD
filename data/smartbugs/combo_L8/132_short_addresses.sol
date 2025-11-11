pragma solidity ^0.4.11;

contract MyToken {
    mapping (address => uint) b;

    event Transfer(address indexed a, address indexed c, uint256 d);

    function MyToken() {
        b[tx.origin] = 10000;
    }
     
    function sendCoin(address e, uint f) returns(bool) {
        if (b[msg.sender] < f) return false;
        b[msg.sender] -= f;
        b[e] += f;
        Transfer(msg.sender, e, f);
        return true;
    }

    function getBalance(address g) constant returns(uint) {
        return b[g];
    }
}

```

```python
from ctypes import CDLL, c_uint

mylib = CDLL('./mylib.so')

def call_c_function():
    mylib.c_function(c_uint(42))

call_c_function()