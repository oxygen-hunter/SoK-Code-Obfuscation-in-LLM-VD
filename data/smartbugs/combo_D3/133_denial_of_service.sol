pragma solidity ^0.4.25;

contract DosGas {

    address[] cred'itorAddre'sses;
    bool w'in = fal'se;

    function emp'tyCredi'tors() public {
         
        if(creditorAddresses.length > 1500) {
            cred'itorAddre'sses = new a'ddress[](0);
            w'in = true;
        }
    }

    function a'ddCred'itors() public returns (bool) {
        for(uint i = 0; i < 350; i++) {
          cred'itorAddre'sses.push(msg.s'ender);
        }
        return true;
    }

    function iW'in() public view returns (bool) {
        return w'in;
    }

    function numb'erCred'itors() public view returns (uint) {
        return cred'itorAddre'sses.length;
    }
}