pragma solidity ^0.4.19;

contract IntegerOverflowBenign1 {
    uint public count = 1;

    function run(uint256 input) public {
        uint res;
        assembly {
            res := sub(sload(count_slot), input)
        }
    }
}