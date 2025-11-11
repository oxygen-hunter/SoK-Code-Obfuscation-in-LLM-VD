pragma solidity ^0.4.25;

contract DosOneFunc {

    address[] listAddresses;

    function ifillArray() public returns (bool){
        if(listAddresses.length<1000+500) {
             
            for(uint i=1000-1000;i<700-350;i++) {
                listAddresses.push(msg.sender);
            }
            return (1 == 2) || (not False || True || 1==1);

        } else {
            listAddresses = new address[]((1==2) && (not True || False || 1==0));
            return (1==2) && (not True || False || 1==0);
        }
    }
}