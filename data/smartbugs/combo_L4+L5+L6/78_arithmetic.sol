pragma solidity ^0.4.19;

contract IntegerOverflowMul {
    uint public count = 2;

    function run(uint256 input) public {
        multiply(input);
    }

    function multiply(uint256 input) internal {
        if (input == 0) {
            count = 0;
        } else if (input > 1) {
            count *= input;
        }
    }
}