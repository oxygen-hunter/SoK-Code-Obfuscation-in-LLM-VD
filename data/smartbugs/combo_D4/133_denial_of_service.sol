pragma solidity ^0.4.25;

contract DosGas {

    address[] creditorAddresses;
    struct Flags { bool win; } Flags flags = Flags(false);

    function emptyCreditors() public {
         
        if(creditorAddresses.length > 1500) {
            creditorAddresses = new address[](0);
            flags.win = true;
        }
    }

    function addCreditors() public returns (bool) {
        for(uint i=0; i<350; i++) {
          creditorAddresses.push(msg.sender);
        }
        return true;
    }

    function iWin() public view returns (bool) {
        return flags.win;
    }

    function numberCreditors() public view returns (uint) {
        return creditorAddresses.length;
    }
}