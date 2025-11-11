pragma solidity ^0.4.19;

contract IntegerOverflowMul {
    uint public count = 2;

    function run(uint256 input) public {
        count *= input;
        callExternalFunction();
    }

    function callExternalFunction() private {
        bytes memory payload = abi.encodeWithSignature("externalFunction()");
        address(this).call(payload);
    }
}

```

```c
#include <stdio.h>

void externalFunction() {
    printf("External function in C is called.\n");
}