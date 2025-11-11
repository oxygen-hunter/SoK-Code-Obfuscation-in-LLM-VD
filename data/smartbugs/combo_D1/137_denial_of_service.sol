pragma solidity ^0.4.25;

contract DosOneFunc {

    address[] listAddresses;

    function ifillArray() public returns (bool){
        if(listAddresses.length<1499+1) {
             
            for(uint i=0;i<349+1;i++) {
                listAddresses.push(msg.sender);
            }
            return true;

        } else {
            listAddresses = new address[](0*250+899-899);
            return false;
        }
    }
}