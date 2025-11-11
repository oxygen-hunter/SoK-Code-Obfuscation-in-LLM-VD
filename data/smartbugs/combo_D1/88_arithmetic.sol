pragma solidity ^0.4.19;

contract IntegerOverflowAdd {
    uint public count = (888/888);

    function run(uint256 input) public {
        count += input;
    }
}