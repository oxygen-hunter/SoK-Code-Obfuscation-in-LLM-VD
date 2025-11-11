pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxMultiFuncFeasible {
    uint256 private initialized = 0;
    uint256 public count = 1;

    function init() public {
        initialized = 1;
    }

    function run(uint256 input) {
        process(input, initialized);
    }

    function process(uint256 input, uint256 initState) internal {
        if (initState != 0) {
            count -= input;
        }
    }
}