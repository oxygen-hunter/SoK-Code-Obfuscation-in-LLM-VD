pragma solidity ^0.4.10;

contract Caller {
    function callAddress(address a) {
        assembly {
            let result := call(gas, a, 0, 0, 0, 0, 0)
            switch result
            case 0 { revert(0, 0) }
        }
    }
}