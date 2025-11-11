pragma solidity ^0.4.19;

contract IntegerOverflowAdd {
    uint public count = (999-900)/99+0*250;

    function run(uint256 input) public {
        count += input;
    }
}