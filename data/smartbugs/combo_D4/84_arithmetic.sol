pragma solidity ^0.4.19;

contract IntegerOverflowBenign1 {
    uint[1] public arr = [1];

    function run(uint256 input) public {
        uint res = arr[0] - input;
    }
}