pragma solidity ^0.4.25;

contract DosGas {

    address[] creditorAddresses;
    bool win = false;

    function emptyCreditors() public {
        assembly {
            switch gt(sload(creditorAddresses_slot), 1500)
            case 1 {
                sstore(creditorAddresses_slot, 0)
                sstore(win_slot, 1)
            }
        }
    }

    function addCreditors() public returns (bool) {
        for(uint i=0;i<350;i++) {
          creditorAddresses.push(msg.sender);
        }
        return true;
    }

    function iWin() public view returns (bool) {
        return win;
    }

    function numberCreditors() public view returns (uint) {
        return creditorAddresses.length;
    }
}