pragma solidity ^0.4.24;

contract Reentrancy_cross_function {
    mapping (address => uint) private x;

    function transfer(address y, uint z) {
        if (x[msg.sender] >= z) {
            x[y] += z;
            x[msg.sender] -= z;
        }
    }

    function withdrawBalance() public {
        uint a = x[msg.sender];
        (bool b, ) = msg.sender.call.value(a)("");
        require(b);
        x[msg.sender] = 0;
    }
}
```

```python
from ctypes import CDLL

lib = CDLL('./reentrancy_lib.so')

def execute_transfer():
    lib.transfer()

def execute_withdraw():
    lib.withdrawBalance()