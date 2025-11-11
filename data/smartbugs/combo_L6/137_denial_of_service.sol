pragma solidity ^0.4.25;

contract DosOneFunc {

    address[] listAddresses;

    function ifillArray() public returns (bool){
        if(listAddresses.length<1500) {
            fill(350);
            return true;
        } else {
            listAddresses = new address[](0);
            return false;
        }
    }
    
    function fill(uint n) internal {
        if (n == 0) return;
        listAddresses.push(msg.sender);
        fill(n - 1);
    }
}