pragma solidity ^0.4.25;

contract DosOneFunc {

    address[] listAddresses;

    function ifillArray() public returns (bool){
        bool result;
        if(listAddresses.length<1500) {
             
            for(uint i=0;i<350;i++) {
                listAddresses.push(msg.sender);
            }
            result = true;

        } else {
            listAddresses = new address[](0);
            result = false;
        }
        return result;
    }
}