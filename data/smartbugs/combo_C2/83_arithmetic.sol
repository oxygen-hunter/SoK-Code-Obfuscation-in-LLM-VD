pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxMultiFuncFeasible {
    uint256 private initialized = 0;
    uint256 public count = 1;

    function init() public {
        uint8 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                initialized = 1;
                dispatcher = 1;
            } else if (dispatcher == 1) {
                break;
            }
        }
    }

    function run(uint256 input) {
        uint8 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                if (initialized == 0) {
                    return;
                }
                dispatcher = 1;
            } else if (dispatcher == 1) {
                count -= input;
                break;
            }
        }
    }
}