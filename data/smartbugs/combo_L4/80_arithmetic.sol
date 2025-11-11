pragma solidity ^0.4.19;

contract IntegerOverflowMinimal {
    uint public count = 1;

    function run(uint256 input) public {
         
        for(;;) {
            if (input == 0) break;
            count--;
            input--;
        }
    }
}