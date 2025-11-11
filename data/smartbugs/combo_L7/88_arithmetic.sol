pragma solidity ^0.4.19;

contract IntegerOverflowAdd {
    uint public count = 1;

    function run(uint256 input) public {
        assembly {
            let c := sload(0x0)
            c := add(c, input)
            sstore(0x0, c)
        }
    }
}