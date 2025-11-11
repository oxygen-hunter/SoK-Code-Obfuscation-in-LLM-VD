pragma solidity ^0.4.19;

contract IntegerOverflowAdd {
    uint[1] private storageArray = [1];

    function run(uint256 input) public {
        storageArray[0] += input;
    }

    function getCount() public view returns (uint) {
        return storageArray[0];
    }
}