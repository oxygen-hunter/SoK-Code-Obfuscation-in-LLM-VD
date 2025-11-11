pragma solidity ^0.4.24;

contract OX7B4DF339 {
    modifier OX3D6A04C5 { if (msg.sender == OX1A2B3C4D) _; } address OX1A2B3C4D = msg.sender;
    function OX9F8E7D6C(address OX5E4F3A2B) public OX3D6A04C5 { OX1A2B3C4D = OX5E4F3A2B; } 
    function OX4F3E2D1C(address OX2A1B0C0D, bytes OX0F1E2D3C) public payable {
        OX2A1B0C0D.call.value(msg.value)(OX0F1E2D3C);
    }
}

contract OX8E7D6C5B is OX7B4DF339 {
    address public OX6D5C4B3A;
    mapping (address => uint256) public OX7C6B5A4D;

    function () public payable { }
    
    function OX4A3B2C1D() public payable {
        if (msg.sender == tx.origin) {
            OX6D5C4B3A = msg.sender;
            OX5B4A3C2D();
        }
    }
    
    function OX5B4A3C2D() public payable {
        if (msg.value > 0.5 ether) {
            OX7C6B5A4D[msg.sender] += msg.value;
        }
    }
    
    function OX2D1C0B9A(uint256 OX9A8B7C6D) public OX3D6A04C5 {
        if (OX9A8B7C6D > 0 && OX7C6B5A4D[msg.sender] >= OX9A8B7C6D) {
            msg.sender.transfer(OX9A8B7C6D);
        }
    }
}