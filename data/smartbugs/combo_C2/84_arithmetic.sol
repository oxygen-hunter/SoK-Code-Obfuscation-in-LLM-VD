pragma solidity ^0.4.19;

contract IntegerOverflowBenign1 {
    uint public count = 1;

    function run(uint256 input) public {
        uint state = 0;
        uint res;
        while(true) {
            if(state == 0) {
                res = count - input;
                state = 1;
            } 
            if(state == 1) {
                break;
            }
        }
    }
}