pragma solidity ^0.4.15;

contract Overflow {
    uint private sellerBalance=0;

    function add(uint value) returns (bool){
        assembly {
            let temp := sload(sellerBalance_slot)
            temp := add(temp, value)
            sstore(sellerBalance_slot, temp)
        }
    }
}