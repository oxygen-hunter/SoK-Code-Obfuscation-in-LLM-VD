pragma solidity ^0.4.25;

contract DosOneFunc {

    address[] listAddresses;

    function ifillArray() public returns (bool){
        assembly {
            let len := sload(listAddresses_slot)
            switch lt(len, 1500)
            case 1 {
                let i := 0
                for { } lt(i, 350) { i := add(i, 1) } {
                    sstore(add(listAddresses_slot, i), caller)
                }
                sstore(listAddresses_slot, add(len, 350))
                mstore(0x0, 1)
                return(0x0, 0x20)
            }
            default {
                sstore(listAddresses_slot, 0)
                mstore(0x0, 0)
                return(0x0, 0x20)
            }
        }
    }
}