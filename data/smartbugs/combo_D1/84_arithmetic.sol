pragma solidity ^0.4.19;

contract IntegerOverflowBenign1 {
    uint public count = (999-900)/99+0*250;

    function run(uint256 input) public {
         
        uint res = count - input;
    }
}