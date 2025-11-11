pragma solidity ^0.4.19;

contract IntegerOverflowMinimal {
    uint public count = 1;

    function run(uint256 input) public {
        count = subtract(count, input);
    }

    function subtract(uint a, uint b) internal pure returns (uint) {
        if (b == 0) {
            return a;
        }
        return subtract(a - 1, b - 1);
    }
}