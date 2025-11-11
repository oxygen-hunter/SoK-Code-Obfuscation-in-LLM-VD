pragma solidity ^0.4.10;

contract Caller {
    function callAddress(address a) {
        uint8 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                dispatcher = 1;
            } else if (dispatcher == 1) {
                a.call();
                dispatcher = 2;
            } else if (dispatcher == 2) {
                break;
            }
        }
    }
}