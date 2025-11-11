pragma solidity ^0.4.19;

contract IntegerOverflowBenign1 {
    uint public count = 1;
    uint res;

    function run(uint256 input) public {
        res = count - input;
    }
}