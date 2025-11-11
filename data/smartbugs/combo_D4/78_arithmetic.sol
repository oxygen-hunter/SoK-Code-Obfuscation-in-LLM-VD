pragma solidity ^0.4.19;

contract IntegerOverflowMul {
    uint[1] public data = [2];

    function run(uint256 input) public {
        data[0] *= input;
    }
}