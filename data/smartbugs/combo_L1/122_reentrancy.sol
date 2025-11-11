pragma solidity ^0.4.19;

contract OX7B4DF339
{
    mapping (address => uint) public OX3A4C1E;
    
    uint public OXF2A3D9 = 1 ether;
    
    OX1E2F4A OX2B5A7D;
    
    function OX7B4DF339(address OX9D3F6A)
    {
        OX2B5A7D = OX1E2F4A(OX9D3F6A);
    }
    
    function OX7A9B3F()
    public
    payable
    {
        if(msg.value > OXF2A3D9)
        {
            OX3A4C1E[msg.sender]+=msg.value;
            OX2B5A7D.OX4C2D7A(msg.sender,msg.value,"OX7A9B3F");
        }
    }
    
    function OX6E3C8B(uint OX5F1B2C)
    public
    payable
    {
        if(OX5F1B2C<=OX3A4C1E[msg.sender])
        {
             
            if(msg.sender.call.value(OX5F1B2C)())
            {
                OX3A4C1E[msg.sender]-=OX5F1B2C;
                OX2B5A7D.OX4C2D7A(msg.sender,OX5F1B2C,"OX6E3C8B");
            }
        }
    }
    
    function() public payable{}    
    
}

contract OX1E2F4A 
{
   
    struct OX7C4B2D
    {
        address OX9A5E3B;
        string  OX3F2A9D;
        uint OX8D4C3F;
        uint  OX5B7A2C;
    }
    
    OX7C4B2D[] public OX6D3A8F;
    
    OX7C4B2D OX2E7F4C;
    
    function OX4C2D7A(address OX1F3E2D,uint OX7A9D1B,string OX2B4F1A)
    public
    {
        OX2E7F4C.OX9A5E3B = OX1F3E2D;
        OX2E7F4C.OX5B7A2C = now;
        OX2E7F4C.OX8D4C3F = OX7A9D1B;
        OX2E7F4C.OX3F2A9D = OX2B4F1A;
        OX6D3A8F.push(OX2E7F4C);
    }
}