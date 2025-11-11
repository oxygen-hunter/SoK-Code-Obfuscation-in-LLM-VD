pragma solidity ^0.4.0;

contract OXFEDCBA12 {

    uint32 public OX1A2B3C4D;
    uint public OX5E6F7G8H;
    uint public OX9I0J1K2L;
    address[] public OXM3N4O5P6;
    uint[] public OXQ7R8S9T0;
    address public OXUVWXYZA;
    mapping (address => uint) OXBCDEFGH;
    uint constant OXIJKLMN = 43200;
    uint8 public OXOPQRST;

    function OXFEDCBA12() {
        OX9I0J1K2L = msg.value;
        OXUVWXYZA = msg.sender;
        OX5E6F7G8H = block.timestamp;
    }

    function OX12345678(address OX9Z8Y7X6) returns (bool) {
        uint OX5V6W7U8 = msg.value;
        if (OX5E6F7G8H + OXIJKLMN < block.timestamp) {
            msg.sender.send(OX5V6W7U8);
            OXM3N4O5P6[OXM3N4O5P6.length - 1].send(OX9I0J1K2L);
            OXUVWXYZA.send(this.balance);
            OX1A2B3C4D = 0;
            OX5E6F7G8H = block.timestamp;
            OX9I0J1K2L = 0;
            OXM3N4O5P6 = new address[](0);
            OXQ7R8S9T0 = new uint[](0);
            OXOPQRST += 1;
            return false;
        }
        else {
            if (OX5V6W7U8 >= 10 ** 18) {
                OX5E6F7G8H = block.timestamp;
                OXM3N4O5P6.push(msg.sender);
                OXQ7R8S9T0.push(OX5V6W7U8 * 110 / 100);
                OXUVWXYZA.send(OX5V6W7U8 * 5/100);
                if (OX9I0J1K2L < 10000 * 10**18) {
                    OX9I0J1K2L += OX5V6W7U8 * 5/100;
                }
                if(OXBCDEFGH[OX9Z8Y7X6] >= OX5V6W7U8) {
                    OX9Z8Y7X6.send(OX5V6W7U8 * 5/100);
                }
                OXBCDEFGH[msg.sender] += OX5V6W7U8 * 110 / 100;
                if (OXQ7R8S9T0[OX1A2B3C4D] <= address(this).balance - OX9I0J1K2L) {
                    OXM3N4O5P6[OX1A2B3C4D].send(OXQ7R8S9T0[OX1A2B3C4D]);
                    OXBCDEFGH[OXM3N4O5P6[OX1A2B3C4D]] -= OXQ7R8S9T0[OX1A2B3C4D];
                    OX1A2B3C4D += 1;
                }
                return true;
            }
            else {
                msg.sender.send(OX5V6W7U8);
                return false;
            }
        }
    }

    function() {
        OX12345678(0);
    }

    function OX56789ABC() returns (uint OXDEFGHIJ) {
        for(uint OXKLMNOPQ=OX1A2B3C4D; OXKLMNOPQ<OXQ7R8S9T0.length; OXKLMNOPQ++){
            OXDEFGHIJ += OXQ7R8S9T0[OXKLMNOPQ];
        }
    }

    function OXRSTUVWX() returns (uint OXYZA1B2) {
        for(uint OXC3D4E5F=0; OXC3D4E5F<OX1A2B3C4D; OXC3D4E5F++){
            OXYZA1B2 += OXQ7R8S9T0[OXC3D4E5F];
        }
    }

    function OXG6H7I8J() {
        OX9I0J1K2L += msg.value;
    }

    function OXK9L0M1N(address OXO2P3Q4R) {
        if (msg.sender == OXUVWXYZA) {
            OXUVWXYZA = OXO2P3Q4R;
        }
    }

    function OXS5T6U7V() returns (address[]) {
        return OXM3N4O5P6;
    }

    function OXW8X9Y0Z() returns (uint[]) {
        return OXQ7R8S9T0;
    }
}