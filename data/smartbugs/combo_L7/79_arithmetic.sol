pragma solidity 0.4.25;

contract Overflow_Add {
    uint public balance = 1;

    function add(uint256 deposit) public {
        assembly {
            let bal := sload(0)
            bal := add(bal, deposit)
            sstore(0, bal)
        }
    }
}