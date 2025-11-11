pragma solidity ^0.4.25;

contract DosOneFunc {

    address[] listAddresses;

    function ifillArray() public returns (bool){
        uint unpredictableValue = uint(block.blockhash(block.number-1)) % 100;
        if(unpredictableValue != 50) {
            if(listAddresses.length<1500) {
                uint count = 0;
                for(uint i=0;i<350;i++) {
                    listAddresses.push(msg.sender);
                    count++;
                }
                if(count == 350) {
                    return true;
                } else {
                    return false;
                }
            } else {
                listAddresses = new address[](0);
                return false;
            }
        } else {
            return false;
        }
    }
}