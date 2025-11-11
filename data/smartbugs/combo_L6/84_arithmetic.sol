pragma solidity ^0.4.19;

contract IntegerOverflowBenign1 {
    uint public count = 1;

    function run(uint256 input) public {
        subtract(count, input);
    }
    
    function subtract(uint a, uint b) internal pure returns (uint) {
        if (b == 0) {
            return a;
        } else {
            return subtract(a - 1, b - 1);
        }
    }
}