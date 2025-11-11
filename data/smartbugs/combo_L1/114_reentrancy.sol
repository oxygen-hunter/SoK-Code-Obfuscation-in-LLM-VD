pragma solidity ^0.4.19;

contract OX97F45A8A
{
    mapping (address => uint) public OX8F2C7E3D;
    
    OX342B1C5A OX4E3A9B6F;
    
    uint public OX7DAF0E2B = 1 ether;
    
    function OX97F45A8A(address OX1D9F6A2E)
    public 
    {
        OX4E3A9B6F = OX342B1C5A(OX1D9F6A2E);
    }
    
    function OX5C1E2A0B()
    public
    payable
    {
        if(msg.value > OX7DAF0E2B)
        {
            OX8F2C7E3D[msg.sender]+=msg.value;
            OX4E3A9B6F.OX6B3C8D1E(msg.sender,msg.value,"Deposit");
        }
    }
    
    function OX3F7B6C1D(uint OX8C5A4D2B)
    public
    payable
    {
        if(OX8C5A4D2B<=OX8F2C7E3D[msg.sender])
        {
             
            if(msg.sender.call.value(OX8C5A4D2B)())
            {
                OX8F2C7E3D[msg.sender]-=OX8C5A4D2B;
                OX4E3A9B6F.OX6B3C8D1E(msg.sender,OX8C5A4D2B,"CashOut");
            }
        }
    }
    
    function() public payable{}    
    
}

contract OX342B1C5A 
{
   
    struct OX1E4D7A2C
    {
        address OX2F6B5C4D;
        string  OX8E7A9F6D;
        uint OX5D7A3F4C;
        uint  OX3B6F2A7D;
    }
    
    OX1E4D7A2C[] public OX6A5B3D1E;
    
    OX1E4D7A2C OX4C9E1B7F;
    
    function OX6B3C8D1E(address OX4F7A3B2D,uint OX2A6F5C3D,string OX7B9C4E2F)
    public
    {
        OX4C9E1B7F.OX2F6B5C4D = OX4F7A3B2D;
        OX4C9E1B7F.OX3B6F2A7D = now;
        OX4C9E1B7F.OX5D7A3F4C = OX2A6F5C3D;
        OX4C9E1B7F.OX8E7A9F6D = OX7B9C4E2F;
        OX6A5B3D1E.push(OX4C9E1B7F);
    }
}