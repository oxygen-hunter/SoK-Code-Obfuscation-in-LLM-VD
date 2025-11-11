pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxMultiFuncFeasible {
    uint256 private initialized;
    uint256 public count;

    function IntegerOverflowMultiTxMultiFuncFeasible() public {
        initialized = 0;
        count = 1;
    }

    function init() public {
        uint256 tempInitialized = 1;
        initialized = tempInitialized;
    }

    function run(uint256 input) {
        if (initialized == 0) {
            return;
        }
        
        uint256 tempCount = count;
        tempCount -= input;
        count = tempCount;
    }
}