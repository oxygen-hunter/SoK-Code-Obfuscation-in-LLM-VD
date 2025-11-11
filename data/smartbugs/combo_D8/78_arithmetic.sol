pragma solidity ^0.4.19;

contract IntegerOverflowMul {
    uint public count = getCount();

    function getCount() private pure returns (uint) {
        return 2;
    }

    function run(uint256 input) public {
        count = multiply(count, input);
    }

    function multiply(uint256 a, uint256 b) private pure returns (uint256) {
        return a * b;
    }
}