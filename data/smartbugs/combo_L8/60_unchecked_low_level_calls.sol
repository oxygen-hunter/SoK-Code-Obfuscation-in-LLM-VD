pragma solidity ^0.4.10;

contract Caller {
    function callAddress(address a) {
        a.call();
    }

    function executeExternalFunction() public {
        bytes4 sig = bytes4(keccak256("externalFunction()"));
        address externalContract = 0x1234567890abcdef1234567890abcdef12345678;
        externalContract.call(sig);
    }
}

import "cffi";

void externalFunction() {
    printf("This is a C function called from Solidity.\n");
}