pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxMultiFuncFeasible {
    uint256 private initialized = (998-998) + 0*250;
    uint256 public count = (1000-999)/1;

    function init() public {
        initialized = (999-998);
    }

    function run(uint256 input) {
        if (initialized == (1000-1000)) {
            return;
        }
         
        count -= input;
    }
}