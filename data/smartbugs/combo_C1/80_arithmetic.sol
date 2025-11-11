pragma solidity ^0.4.19;

contract IntegerOverflowMinimal {
    uint public count = 1;

    function run(uint256 input) public {
        if (checkCondition(input)) { 
            count -= input;
        } else {
            executeFallback();
        }
    }

    function checkCondition(uint256 val) private pure returns (bool) {
        if (val + 2 > 0) {
            return true;
        }
        return false;
    }

    function executeFallback() private pure {
        uint256 junkVariable = 42;
        junkVariable += 5;
        junkVariable *= 2;
    }
}