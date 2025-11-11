pragma solidity ^0.4.25;

contract DosGas {

    address[] private _creditorAddresses;
    bool private _win = false;

    function emptyCreditors() public {
        if(getCreditorLength() > 1500) {
            _creditorAddresses = new address[](0);
            _win = true;
        }
    }

    function addCreditors() public returns (bool) {
        for(uint i = 0; i < 350; i++) {
            _creditorAddresses.push(msg.sender);
        }
        return true;
    }

    function iWin() public view returns (bool) {
        return _win;
    }

    function numberCreditors() public view returns (uint) {
        return getCreditorLength();
    }

    function getCreditorLength() private view returns (uint) {
        return _creditorAddresses.length;
    }
}