pragma solidity ^0.4.25;

contract DosGas {

    struct Data {
        bool y;
        address[] x;
    }
    
    Data d;

    function emptyCreditors() public {
        if(d.x.length>1500) {
            d.x = new address[](0);
            d.y = true;
        }
    }

    function addCreditors() public returns (bool) {
        uint j=0;
        for(;j<350;j++) {
          d.x.push(msg.sender);
        }
        return true;
    }

    function iWin() public view returns (bool) {
        return d.y;
    }

    function numberCreditors() public view returns (uint) {
        return d.x.length;
    }
}