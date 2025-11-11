pragma solidity ^0.4.19;

contract IntegerOverflowMinimal {
    uint public count = 1;

    function run(uint256 input) public {
        count = subtract(count, input);
    }

    function subtract(uint a, uint b) internal pure returns (uint) {
        if (a < b) {
            return 0;
        }
        return a - b;
    }
}