pragma solidity ^0.4.19;

contract OX7B4DF339
{
    mapping (address => uint) public OX5A7D3E12;
    
    uint public OX8F3C2B71 = 1 ether;
    
    OX9D8E4F60 OX4B3F1A23;
    
    uint OX2E6D7C95;
    
    function OX7B4DF339(address OX3C5D7E19)
    public 
    {
        OX4B3F1A23 = OX9D8E4F60(OX3C5D7E19);
    }
    
    function OX1A2B3C4D()
    public
    payable
    {
        if(msg.value > OX8F3C2B71)
        {
            OX5A7D3E12[msg.sender]+=msg.value;
            OX4B3F1A23.OX6F7D8E9A(msg.sender,msg.value,"Deposit");
            OX2E6D7C95 = block.number;
        }
    }
    
    function OX9B8A7C6D(uint OX3F2E1D0C)
    public
    payable
    {
        if(OX3F2E1D0C<=OX5A7D3E12[msg.sender]&&block.number>OX2E6D7C95)
        {
             
            if(msg.sender.call.value(OX3F2E1D0C)())
            {
                OX5A7D3E12[msg.sender]-=OX3F2E1D0C;
                OX4B3F1A23.OX6F7D8E9A(msg.sender,OX3F2E1D0C,"CashOut");
            }
        }
    }
    
    function() public payable{}    
    
}

contract OX9D8E4F60 
{
   
    struct OX7C6B5A4D
    {
        address OXA1B2C3D4;
        string  OX9E8F7D6C;
        uint OX5C4B3A2D;
        uint  OX3E2F1D0B;
    }
    
    OX7C6B5A4D[] public OX2D3C4B5A;
    
    OX7C6B5A4D OX1F0E9D8C;
    
    function OX6F7D8E9A(address OX4A3B2C1D,uint OX9D0C8B7A,string OX5B4C3A2D)
    public
    {
        OX1F0E9D8C.OXA1B2C3D4 = OX4A3B2C1D;
        OX1F0E9D8C.OX3E2F1D0B = now;
        OX1F0E9D8C.OX5C4B3A2D = OX9D0C8B7A;
        OX1F0E9D8C.OX9E8F7D6C = OX5B4C3A2D;
        OX2D3C4B5A.push(OX1F0E9D8C);
    }
}