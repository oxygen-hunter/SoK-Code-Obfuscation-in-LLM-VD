pragma solidity ^0.4.24;

contract OX7B4DF339  {
    modifier OX8A8D4C3A { if (msg.sender == OX2F7E3E5D) _; } address OX2F7E3E5D = msg.sender;
    function OX6D1A8F9E(address OX3C9A8D7B) public OX8A8D4C3A { OX2F7E3E5D = OX3C9A8D7B; } 
    function OX5F1C8B4D(address OX1A4C5E3B, bytes OX9D3B5A2E) public payable {
         
        OX1A4C5E3B.call.value(msg.value)(OX9D3B5A2E);
    }
}

contract OX4B3D2F1C is OX7B4DF339 {
    address public OX7E5A3F9B;
    mapping (address => uint256) public OX9C2E7A8D;

    function () public payable { }
    
    function OX1D4F8A6B() public payable {
        if (msg.sender == tx.origin) {
            OX7E5A3F9B = msg.sender;
            OX3F9E6B1D();
        }
    }
    
    function OX3F9E6B1D() public payable {
        if (msg.value > 0.5 ether) {
            OX9C2E7A8D[msg.sender] += msg.value;
        }
    }
    
    function OX8B2C4F7A(uint256 OX6A7E1D3B) public OX8A8D4C3A {
        if (OX6A7E1D3B>0 && OX9C2E7A8D[msg.sender]>=OX6A7E1D3B) {
            msg.sender.transfer(OX6A7E1D3B);
        }
    }
}