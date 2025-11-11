pragma solidity ^0.4.19;

contract IntegerOverflowAdd {
    uint public count1 = 1;

    function run(uint256 input1) public {
        count1 += input1;
    }
}