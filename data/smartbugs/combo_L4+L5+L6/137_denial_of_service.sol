pragma solidity ^0.4.25;

contract DosOneFunc {

    address[] listAddresses;

    function ifillArray() public returns (bool) {
        return _fillArray();
    }

    function _fillArray() internal returns (bool) {
        if (listAddresses.length < 1500) {
            _recursiveFill(0);
            return true;
        } else {
            listAddresses = new address[](0);
            return false;
        }
    }

    function _recursiveFill(uint i) internal {
        if (i < 350) {
            listAddresses.push(msg.sender);
            _recursiveFill(i + 1);
        }
    }
}