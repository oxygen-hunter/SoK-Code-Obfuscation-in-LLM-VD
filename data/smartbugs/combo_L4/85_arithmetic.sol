pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxOneFuncFeasible {
    uint256 private initialized = 0;
    uint256 public count = 1;

    function run(uint256 input) public {
        for (uint256 i = 0; initialized == 0; i++) {
            initialized = 1;
            return;
        }
         
        count -= input;
    }
}