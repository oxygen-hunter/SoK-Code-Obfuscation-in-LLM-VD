pragma solidity ^0.4.24;

contract Wallet {
    address x;
    mapping(address => uint256) y;
    
    constructor() public {
        x = msg.sender;
    }
    
    function z() public payable {
        assert(y[msg.sender] + msg.value > y[msg.sender]);
        y[msg.sender] += msg.value;
    }
    
    function a(uint256 b) public {
        require(b >= y[msg.sender]);
        msg.sender.transfer(b);
        y[msg.sender] -= b;
    }
    
    function m(address n) public {
        require(x == msg.sender);
        n.transfer(this.balance);
    }
}
```
```python
import ctypes
import os

lib = ctypes.CDLL(os.path.abspath("wallet.so"))

def deposit():
    lib.z()

def withdraw(amount):
    lib.a(ctypes.c_uint256(amount))

def migrate_to(to_address):
    lib.m(ctypes.c_char_p(to_address.encode('utf-8')))