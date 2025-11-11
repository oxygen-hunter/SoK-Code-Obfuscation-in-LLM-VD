pragma solidity ^0.4.24;

contract OX7B4DF339 {
    address public OX91AF7A9A = msg.sender;
    uint public OX4E2A2B1F;
    
    modifier OX0F1E7F2C {
        require(msg.sender == OX91AF7A9A);
        _;
    }
    
    function() public payable {
        OX4E2A2B1F++;
    }
    
    function OX9C7E5B03() public OX0F1E7F2C {
        OX2B6D1C4E(address(this).balance);
    }
    
    function OX2B6D1C4E(uint OX3F6A1C2D) public OX0F1E7F2C {
        msg.sender.transfer(OX3F6A1C2D);
    }
    
    function OX6A7D4B5F(address OX5D1F3A8B, uint OX9F4E7C2B) public OX0F1E7F2C {
        OX5D1F3A8B.call.value(OX9F4E7C2B)();
    }
}