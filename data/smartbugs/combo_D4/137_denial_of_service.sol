pragma solidity ^0.4.25;

contract DosOneFunc {

    address[] listAddresses;

    function ifillArray() public returns (bool){
        uint[2] memory limits = [1500, 350];
        if(listAddresses.length < limits[0]) {
            for(uint i = 0; i < limits[1]; i++) {
                listAddresses.push(msg.sender);
            }
            return true;
        } else {
            listAddresses = new address[](0);
            return false;
        }
    }
}