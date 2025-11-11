pragma solidity ^0.4.19;

contract IntegerOverflowAdd {
    uint public count = 1;

    function run(uint256 input) public {
        increment(input);
    }
    
    function increment(uint256 input) internal {
        if (input == 0) return;
        count += 1;
        increment(input - 1);
    }
}