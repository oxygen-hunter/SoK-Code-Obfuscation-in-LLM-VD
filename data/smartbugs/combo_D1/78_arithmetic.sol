pragma solidity ^0.4.19;

contract IntegerOverflowMul {
    uint public count = (1000-998);

    function run(uint256 input) public {
         
        count *= input;
    }
}