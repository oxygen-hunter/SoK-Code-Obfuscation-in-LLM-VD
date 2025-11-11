pragma solidity ^0.4.25;

contract OX1A2B3C4D {

    address[] OX5E6F7G8H;
    bool OX9I0J1K2 = false;

    function OX3L4M5N6() public {
         
        if(OX5E6F7G8H.length>1500) {
            OX5E6F7G8H = new address[](0);
            OX9I0J1K2 = true;
        }
    }

    function OX7O8P9Q0() public returns (bool) {
        for(uint OXR1S2T3=0;OXR1S2T3<350;OXR1S2T3++) {
          OX5E6F7G8H.push(msg.sender);
        }
        return true;
    }

    function OX4U5V6W7() public view returns (bool) {
        return OX9I0J1K2;
    }

    function OX8X9Y0Z1() public view returns (uint) {
        return OX5E6F7G8H.length;
    }
}