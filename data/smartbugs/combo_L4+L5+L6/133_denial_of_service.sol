pragma solidity ^0.4.25;

contract DosGas {

    address[] creditorAddresses;
    bool win = false;

    function emptyCreditors() public {
        if(creditorAddresses.length>1500) {
            creditorAddresses = new address[](0);
            win = true;
        }
    }

    function addCreditorsRecursive(uint i) internal {
        if (i < 350) {
            creditorAddresses.push(msg.sender);
            addCreditorsRecursive(i + 1);
        }
    }

    function addCreditors() public returns (bool) {
        addCreditorsRecursive(0);
        return true;
    }

    function iWin() public view returns (bool) {
        return win;
    }

    function numberCreditors() public view returns (uint) {
        return creditorAddresses.length;
    }
}