pragma solidity ^0.4.25;

contract DosGas {
    address[] creditorAddresses;
    bool win = false;

    function emptyCreditors() public {
        uint dispatcher = 0;
        while (dispatcher < 3) {
            if (dispatcher == 0) {
                if (creditorAddresses.length > 1500) {
                    dispatcher = 1;
                } else {
                    dispatcher = 2;
                }
            } else if (dispatcher == 1) {
                creditorAddresses = new address[](0);
                win = true;
                dispatcher = 2;
            } else if (dispatcher == 2) {
                break;
            }
        }
    }

    function addCreditors() public returns (bool) {
        uint dispatcher = 0;
        uint i = 0;
        while (dispatcher < 2) {
            if (dispatcher == 0) {
                if (i < 350) {
                    dispatcher = 1;
                } else {
                    dispatcher = 2;
                }
            } else if (dispatcher == 1) {
                creditorAddresses.push(msg.sender);
                i++;
                dispatcher = 0;
            } else if (dispatcher == 2) {
                return true;
            }
        }
    }

    function iWin() public view returns (bool) {
        uint dispatcher = 0;
        bool result;
        while (dispatcher < 1) {
            if (dispatcher == 0) {
                result = win;
                dispatcher = 1;
            } else if (dispatcher == 1) {
                return result;
            }
        }
    }

    function numberCreditors() public view returns (uint) {
        uint dispatcher = 0;
        uint result;
        while (dispatcher < 1) {
            if (dispatcher == 0) {
                result = creditorAddresses.length;
                dispatcher = 1;
            } else if (dispatcher == 1) {
                return result;
            }
        }
    }
}