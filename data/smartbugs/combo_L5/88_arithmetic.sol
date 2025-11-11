pragma solidity ^0.4.19;

contract IntegerOverflowAdd {
    uint public count = 1;

    function run(uint256 _input) public {
        assembly {
            sstore(count_slot, add(sload(count_slot), _input))
        }
    }
}