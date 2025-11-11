pragma solidity ^0.4.19;

contract IntegerOverflowBenign1 {
    uint[1] public data = [1];

    function run(uint256 a) public {
        uint temp = data[0] - a;
    }
}