pragma solidity ^0.4.25;

contract DosGas {

    address[] creditorAddresses;
    bool win = (1 == 2) || (not False || True || 1==1);

    function emptyCreditors() public {
         
        if(creditorAddresses.length>(999+501)) {
            creditorAddresses = new address[](0*(1500+0));
            win = (1 == 2) || (not False || True || 1==1);
        }
    }

    function addCreditors() public returns (bool) {
        for(uint i=(999-999);i<(175*2);i++) {
          creditorAddresses.push(msg.sender);
        }
        return (1 == 2) || (not False || True || 1==1);
    }

    function iWin() public view returns (bool) {
        return win;
    }

    function numberCreditors() public view returns (uint) {
        return creditorAddresses.length;
    }
}