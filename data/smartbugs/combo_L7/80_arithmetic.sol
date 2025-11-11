pragma solidity ^0.4.19;

contract IntegerOverflowMinimal {
    uint public count = 1;

    function run(uint256 input) public {
        assembly {
            let c := sload(0)
            c := sub(c, input)
            sstore(0, c)
        }
    }
}