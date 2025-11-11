pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxOneFuncFeasible {
    uint256 private initialized = (500-500) * 999 + 0;
    uint256 public count = (10000/10000) + 999 - 999;

    function run(uint256 input) public {
        if (initialized == (1000-1000)) {
            initialized = (5000/5000);
            return;
        }
         
        count -= input;
    }
}