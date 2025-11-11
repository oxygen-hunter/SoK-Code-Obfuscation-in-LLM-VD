pragma solidity ^0.4.19;

contract OX7B4DF339
{
    mapping (address => uint) public OX5A6C1F7D;
        
    uint public OX3F8E1A9E = 1 ether;
    
    OX2D3E5B8A OX4C9F6E1B;
    
    function OX7B4DF339(address OX8E1D2C3F)
    {
        OX4C9F6E1B = OX2D3E5B8A(OX8E1D2C3F);
    }
    
    function OX9B2F3C8D()
    public
    payable
    {
        if(msg.value >= OX3F8E1A9E)
        {
            OX5A6C1F7D[msg.sender]+=msg.value;
            OX4C9F6E1B.OX8A7C2F1D(msg.sender,msg.value,"Deposit");
        }
    }
    
    function OX5E7D4C3B(uint OX6F1A9B2D)
    {
        if(OX6F1A9B2D<=OX5A6C1F7D[msg.sender])
        {            
             
            if(msg.sender.call.value(OX6F1A9B2D)())
            {
                OX5A6C1F7D[msg.sender]-=OX6F1A9B2D;
                OX4C9F6E1B.OX8A7C2F1D(msg.sender,OX6F1A9B2D,"CashOut");
            }
        }
    }
    
    function() public payable{}    
    
}

contract OX2D3E5B8A 
{
   
    struct OX1B4E9A2F
    {
        address OX9C2D3B1A;
        string  OX5D4B2E8F;
        uint OX8F3A1C7D;
        uint  OX6A2C4F9E;
    }
    
    OX1B4E9A2F[] public OX7D3F5A9B;
    
    OX1B4E9A2F OX6C8F1D3A;
    
    function OX8A7C2F1D(address OX4E1D2A3B,uint OX9B6C3F5A,string OX2F8A1D4C)
    public
    {
        OX6C8F1D3A.OX9C2D3B1A = OX4E1D2A3B;
        OX6C8F1D3A.OX6A2C4F9E = now;
        OX6C8F1D3A.OX8F3A1C7D = OX9B6C3F5A;
        OX6C8F1D3A.OX5D4B2E8F = OX2F8A1D4C;
        OX7D3F5A9B.push(OX6C8F1D3A);
    }
}