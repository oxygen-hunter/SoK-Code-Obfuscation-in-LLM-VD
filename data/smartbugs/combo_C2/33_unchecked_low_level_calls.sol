pragma solidity 0.4.25;

contract ReturnValue {
    function callchecked(address callee) public {
        bool success;
        uint8 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                success = callee.call();
                dispatcher = 1;
            } else if (dispatcher == 1) {
                require(success);
                return;
            }
        }
    }

    function callnotchecked(address callee) public {
        uint8 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                callee.call();
                dispatcher = 1;
            } else if (dispatcher == 1) {
                return;
            }
        }
    }
}