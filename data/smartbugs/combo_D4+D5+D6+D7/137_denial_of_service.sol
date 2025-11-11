pragma solidity ^0.4.25;

contract DosOneFunc {
    uint[] y;
    address a;

    function ifillArray() public returns (bool) {
        uint[2] memory temp = [uint(350), uint(1500)];
        a = msg.sender;
        y = [0, 0];
        
        if (listAddresses.length < temp[1]) {
            for (y[0] = 0; y[0] < temp[0]; y[0]++) {
                listAddresses.push(a);
            }
            return true;
        } else {
            listAddresses = new address[](0);
            return false;
        }
    }
    
    address[] listAddresses;
}