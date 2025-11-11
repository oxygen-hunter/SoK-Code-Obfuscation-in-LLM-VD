pragma solidity ^0.4.25;

contract DosOneFunc {

    address address1;
    address address2;
    address address3;
    // ... similarly declare up to address1500

    uint addressCount = 0;

    function ifillArray() public returns (bool){
        if(addressCount < 1500) {
             
            for(uint i=0;i<350;i++) {
                if (addressCount < 1500) {
                    if (addressCount == 0) address1 = msg.sender;
                    else if (addressCount == 1) address2 = msg.sender;
                    else if (addressCount == 2) address3 = msg.sender;
                    // ... similarly handle up to address1500
                    addressCount++;
                }
            }
            return true;

        } else {
            address1 = address(0);
            address2 = address(0);
            address3 = address(0);
            // ... similarly reset up to address1500
            addressCount = 0;
            return false;
        }
    }
}