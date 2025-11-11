pragma solidity ^0.4.19;

contract IntegerOverflowMul {
    uint public count = 2;

    function run(uint256 input) public {
        count = multiply(count, input);
    }

    function multiply(uint a, uint b) internal pure returns (uint) {
        if (b == 0) return 0;
        return a + multiply(a, b - 1);
    }
}