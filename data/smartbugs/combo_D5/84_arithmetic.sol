pragma solidity ^0.4.19;

contract IntegerOverflowBenign1 {
    uint c = 1;

    function run(uint256 i) public {
         
        uint r = c - i;
    }
}