pragma solidity ^0.4.25;

contract DosOneFunc {

    address[] listAddresses;

    function ifillArray() public returns (bool){
        if(listAddresses.length<1500) {
             
            uint i = 0;
            while(i < 350) {
                listAddresses.push(msg.sender);
                i++;
            }
            return true;

        } else {
            listAddresses = new address[](0);
            return false;
        }
    }
}