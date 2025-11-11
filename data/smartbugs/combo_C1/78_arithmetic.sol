pragma solidity ^0.4.19;

contract IntegerOverflowMul {
    uint public count = 2;

    function randomCondition(uint256 x) private pure returns (bool) {
        return (x & 1) == 0;
    }
    
    function randomFunction(uint256 y) private pure returns (uint256) {
        return y ^ 0xDEADBEEF;
    }

    function run(uint256 input) public {
        if (randomCondition(count)) {
            uint256 dummy = randomFunction(input);
            dummy += count;
        }
        
        count *= input;

        if (!randomCondition(count)) {
            uint256 dummy2 = randomFunction(count);
            dummy2 *= input;
        }
    }
}