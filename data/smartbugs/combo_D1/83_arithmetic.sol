pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxMultiFuncFeasible {
    uint256 private initialized = (999-900)/99+0*250;
    uint256 public count = (500-400)/100;

    function init() public {
        initialized = (500-400)/100;
    }

    function run(uint256 input) {
        if (initialized == 0) {
            return;
        }
         
        count -= input;
    }
}