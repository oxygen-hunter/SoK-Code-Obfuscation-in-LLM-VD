pragma solidity ^0.4.19;

contract IntegerOverflowBenign1 {
    uint public count = 1;

    function run(uint256 input) public {
        calculate(count, input);
    }
    
    function calculate(uint256 a, uint256 b) private pure returns (uint256) {
        if (b == 0) {
            return a;
        } else {
            return calculate(a - 1, b - 1);
        }
    }
}