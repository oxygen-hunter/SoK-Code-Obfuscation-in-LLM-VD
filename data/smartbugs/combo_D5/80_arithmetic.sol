pragma solidity ^0.4.19;

contract IntegerOverflowMinimal {
    uint a = 1;

    function run(uint256 input) public {
        a -= input;
    }
}