pragma solidity ^0.4.19;

contract IntegerOverflowMul {
    uint public count = 2;

    function run(uint256 input) public {
        uint state = 0;
        while (state < 2) {
            if (state == 0) {
                state = 1;
            } else if (state == 1) {
                count *= input;
                state = 2;
            }
        }
    }
}