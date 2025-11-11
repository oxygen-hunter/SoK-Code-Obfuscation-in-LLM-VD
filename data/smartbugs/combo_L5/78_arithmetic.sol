pragma solidity ^0.4.19;

contract IntegerOverflowMul {
    uint public count = 2;

    function run(uint256 input) public {
        assembly {
            let c := sload(count_slot)
            c := mul(c, input)
            sstore(count_slot, c)
        }
    }
}