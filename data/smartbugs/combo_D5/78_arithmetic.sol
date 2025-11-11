pragma solidity ^0.4.19;

contract IntegerOverflowMul {
    uint public c = 2;

    function run(uint256 i) public {
        c *= i;
    }
}