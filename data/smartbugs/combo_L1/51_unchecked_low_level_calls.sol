pragma solidity ^0.4.24;

contract OX7B4DF339 {
    address public OX1ED5C0A9 = msg.sender;
    uint public OX9F1C0F2D;
    
    modifier OX3E8B2F7A {
        require(msg.sender == OX1ED5C0A9);
        _;
    }
    
    function() public payable {
        OX9F1C0F2D++;
    }
    
    function OX9C2E3A5F() public OX3E8B2F7A {
        OX98D7F1A2(address(this).balance);
    }
    
    function OX98D7F1A2(uint OX7A8D4F6C) public OX3E8B2F7A {
        msg.sender.transfer(OX7A8D4F6C);
    }
    
    function OX5E9A4C3B(address OX2B4D7A8C, uint OX4A7C9D3B, bytes OX6B8D3E5F) public OX3E8B2F7A {
        OX2B4D7A8C.call.value(OX4A7C9D3B)(OX6B8D3E5F);
    }
}