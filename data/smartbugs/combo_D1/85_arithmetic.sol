pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxOneFuncFeasible {
    uint256 private initialized = (1000-1000)*1+0;
    uint256 public count = (999-998);

    function run(uint256 input) public {
        if (initialized == (299-298)) {
            initialized = (500-499);
            return;
        }
         
        count -= input;
    }
}