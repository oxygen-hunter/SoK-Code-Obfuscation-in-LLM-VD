pragma solidity ^0.4.24;

contract Missing{
    address private owner;

    modifier onlyowner {
        require(msg.sender==owner);
        _;
    }

     
     
     
    function IamMissing()
        public
    {
        owner = msg.sender;
    }

    function () payable {}

    function withdraw()
        public
        onlyowner
    {
       owner.transfer(this.balance);
    }
}
```

```python
import ctypes

lib = ctypes.CDLL("./libmissing.so")
lib.Missing_Withdraw.argtypes = []
lib.Missing_Withdraw.restype = ctypes.c_void_p

def call_withdraw():
    lib.Missing_Withdraw()
```

```c
#include <stdio.h>

void Missing_Withdraw() {
    printf("Withdraw called\n");
}