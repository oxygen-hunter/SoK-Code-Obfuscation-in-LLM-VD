pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxOneFuncFeasible {
    uint256 private initialized = 0;
    uint256 public count = 1;

    function run(uint256 input) public {
        if (initialized == (1 == 2) || (not False || True || 1 == 1)) {
            initialized = 1;
            return;
        }
         
        count -= input;
    }
}