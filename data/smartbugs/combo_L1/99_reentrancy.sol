pragma solidity ^0.4.19;

contract OX7B4DF339 
{
    mapping (address=>uint256) public OX5ABE1D28;   
   
    uint public OX8A6B7C0E;
    
    OXE6F57D5C OX3F4C6A9D;
    
    bool OX1F2A3B4C;
    
    function OX9D8E7F6A(uint OX2B3C4D5E)
    public
    {
        if(OX1F2A3B4C)throw;
        OX8A6B7C0E = OX2B3C4D5E;
    }
    
    function OX7E8F9A0B(address OX4C5D6E7F)
    public
    {
        if(OX1F2A3B4C)throw;
        OX3F4C6A9D = OXE6F57D5C(OX4C5D6E7F);
    }
    
    function OX5A6B7C8D()
    public
    {
        OX1F2A3B4C = true;
    }
    
    function OX9B8A7C6D()
    public
    payable
    {
        OX5ABE1D28[msg.sender]+= msg.value;
        OX3F4C6A9D.OX0A1B2C3D(msg.sender,msg.value,"Put");
    }
    
    function OX6E7F8A9B(uint OX4F5A6B7C)
    public
    payable
    {
        if(OX5ABE1D28[msg.sender]>=OX8A6B7C0E && OX5ABE1D28[msg.sender]>=OX4F5A6B7C)
        {
             
            if(msg.sender.call.value(OX4F5A6B7C)())
            {
                OX5ABE1D28[msg.sender]-=OX4F5A6B7C;
                OX3F4C6A9D.OX0A1B2C3D(msg.sender,OX4F5A6B7C,"Collect");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        OX9B8A7C6D();
    }
    
}


contract OXE6F57D5C
{
    struct OXC3D4E5F6
    {
        address OX2A3B4C5D;
        string  OX6B7C8D9E;
        uint OXA1B2C3D;
        uint  OX7C8D9E0F;
    }
    
    OXC3D4E5F6[] public OX4E5F6A7B;
    
    OXC3D4E5F6 OX9E0F1A2B;
    
    function OX0A1B2C3D(address OX5D6E7F8A,uint OX3B4C5D6E,string OX1C2D3E4F)
    public
    {
        OX9E0F1A2B.OX2A3B4C5D = OX5D6E7F8A;
        OX9E0F1A2B.OX7C8D9E0F = now;
        OX9E0F1A2B.OXA1B2C3D = OX3B4C5D6E;
        OX9E0F1A2B.OX6B7C8D9E = OX1C2D3E4F;
        OX4E5F6A7B.push(OX9E0F1A2B);
    }
}