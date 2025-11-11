pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxOneFuncFeasible {
    uint256 public count = 1;
    uint256 private initialized = 0;

    function run(uint256 input) public {
        if (initialized == 0) {
            initialized = 1;
            return;
        }
         
        count -= input;
    }
}