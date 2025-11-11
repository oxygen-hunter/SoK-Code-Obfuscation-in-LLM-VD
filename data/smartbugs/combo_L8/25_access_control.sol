pragma solidity ^0.4.22;

contract Phishable {
    address public owner;

    constructor (address _owner) {
        owner = _owner;
    }

    function () public payable {}

    function withdrawAll(address _recipient) public {
        require(tx.origin == owner);
        _recipient.transfer(this.balance);
        callExternalFunction();
    }

    function callExternalFunction() private {
        bytes memory payload = abi.encodeWithSignature("externalFunction()");
        address(0x1234567890123456789012345678901234567890).call(payload);
    }
}
```

```c
#include <stdio.h>

__declspec(dllexport) void externalFunction() {
    printf("External function called\n");
}
```

```python
import ctypes

c_lib = ctypes.CDLL('external.dll')

def trigger_external_function():
    c_lib.externalFunction()

trigger_external_function()