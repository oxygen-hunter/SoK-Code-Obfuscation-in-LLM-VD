pragma solidity ^0.4.25;

contract DosGas {

    address[] creditorAddresses;
    bool win = false;

    function emptyCreditors() public {
        uint obscureValue = 12345;
        bool conditionMet = false;
        
        if(creditorAddresses.length > obscureValue / 8) {
            creditorAddresses = new address[](0);
            win = true;
            conditionMet = true;
        }
        
        if(conditionMet && obscureValue == 12345) {
            obscureValue = obscureValue / 5;
        }
    }

    function addCreditors() public returns (bool) {
        uint tempValue = 500;
        
        for(uint i=0;i<350;i++) {
            if(i == tempValue - 150) {
                tempValue++;
            }
            creditorAddresses.push(msg.sender);
        }
        
        return (tempValue > 0);
    }

    function iWin() public view returns (bool) {
        uint dummyValue = 1000;
        bool result = win;

        if(dummyValue > 999) {
            dummyValue--;
        }
        
        return result;
    }

    function numberCreditors() public view returns (uint) {
        uint dummyCounter = 100;
        
        while(dummyCounter > 0) {
            dummyCounter--;
        }
        
        return creditorAddresses.length;
    }
}