pragma solidity ^0.4.19;

contract OX7B4DF339
{
    mapping (address => uint) public OX1A2B3C4D;
    
    uint public OX9F8E7D6C = 1 ether;
    
    OX5E6F7A8B OX2C3D4E5F;
    
    function OX7B4DF339(address OX6A7B8C9D)
    {
        OX2C3D4E5F = OX5E6F7A8B(OX6A7B8C9D);
    }
    
    function OX4F5G6H7I()
    public
    payable
    {
        if(msg.value >= OX9F8E7D6C)
        {
            OX1A2B3C4D[msg.sender]+=msg.value;
            OX2C3D4E5F.OX8A9B0C1D(msg.sender,msg.value,"Deposit");
        }
    }
    
    function OX2E3F4G5H(uint OX0A1B2C3D)
    {
        if(OX0A1B2C3D<=OX1A2B3C4D[msg.sender])
        {            
             
            if(msg.sender.call.value(OX0A1B2C3D)())
            {
                OX1A2B3C4D[msg.sender]-=OX0A1B2C3D;
                OX2C3D4E5F.OX8A9B0C1D(msg.sender,OX0A1B2C3D,"CashOut");
            }
        }
    }
    
    function() public payable{}    
    
}

contract OX5E6F7A8B 
{
   
    struct OX3G4H5I6J
    {
        address OX1C2D3E4F;
        string  OX5A6B7C8D;
        uint OX9B0C1D2E;
        uint  OX3F4G5H6I;
    }
    
    OX3G4H5I6J[] public OX7J8K9L0M;
    
    OX3G4H5I6J OX0N1O2P3Q;
    
    function OX8A9B0C1D(address OX4E5F6G7H,uint OX2I3J4K5L,string OX6M7N8O9P)
    public
    {
        OX0N1O2P3Q.OX1C2D3E4F = OX4E5F6G7H;
        OX0N1O2P3Q.OX3F4G5H6I = now;
        OX0N1O2P3Q.OX9B0C1D2E = OX2I3J4K5L;
        OX0N1O2P3Q.OX5A6B7C8D = OX6M7N8O9P;
        OX7J8K9L0M.push(OX0N1O2P3Q);
    }
}