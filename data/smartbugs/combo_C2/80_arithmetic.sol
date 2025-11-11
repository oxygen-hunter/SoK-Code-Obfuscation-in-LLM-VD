pragma solidity ^0.4.19;

contract IntegerOverflowMinimal {
    uint public count = 1;

    function run(uint256 input) public {
        bool finished = false;
        uint8 state = 0;
        
        while (!finished) {
            if (state == 0) {
                state = 1;
            } else if (state == 1) {
                count -= input;
                state = 2;
            } else if (state == 2) {
                finished = true;
            }
        }
    }
}