pragma solidity ^0.4.19;

contract IntegerOverflowAdd {
    uint[1] public data = [1];

    function run(uint256 input) public {
        data[0] += input;
    }
}