pragma solidity ^0.4.19;

contract OX3B7A61F9  
{
    struct OX9C5B4D8A   
    {
        uint OX6D3B2F5C;
        uint OX2A1E4B9F;
    }
    
    mapping (address => OX9C5B4D8A) public OX7F8D4C3E;
    
    uint public OX1C2E3B4A;
    
    OX8E9F1C0D OX0A4B6C8D;
    
    bool OX5D1E3F7A;
    
    function OX2E4C6B9D(uint OX4A3B5E1F)
    public
    {
        if(OX5D1E3F7A)throw;
        OX1C2E3B4A = OX4A3B5E1F;
    }
    
    function OX3C1A6F5E(address OX7D9E4B1C)
    public
    {
        if(OX5D1E3F7A)throw;
        OX0A4B6C8D = OX8E9F1C0D(OX7D9E4B1C);
    }
    
    function OX6F2D9C1B()
    public
    {
        OX5D1E3F7A = true;
    }
    
    function OX9B3D1A6C(uint OX5C7E3F1A)
    public
    payable
    {
        var OX4D1F8B7C = OX7F8D4C3E[msg.sender];
        OX4D1F8B7C.OX2A1E4B9F += msg.value;
        if(now+OX5C7E3F1A>OX4D1F8B7C.OX6D3B2F5C)OX4D1F8B7C.OX6D3B2F5C=now+OX5C7E3F1A;
        OX0A4B6C8D.OX4E7A9C3D(msg.sender,msg.value,"Put");
    }
    
    function OX1F2B3C4D(uint OX7E5A9F3B)
    public
    payable
    {
        var OX6A3D5F1C = OX7F8D4C3E[msg.sender];
        if( OX6A3D5F1C.OX2A1E4B9F>=OX1C2E3B4A && OX6A3D5F1C.OX2A1E4B9F>=OX7E5A9F3B && now>OX6A3D5F1C.OX6D3B2F5C)
        {
             
            if(msg.sender.call.value(OX7E5A9F3B)())
            {
                OX6A3D5F1C.OX2A1E4B9F-=OX7E5A9F3B;
                OX0A4B6C8D.OX4E7A9C3D(msg.sender,OX7E5A9F3B,"Collect");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        OX9B3D1A6C(0);
    }
    
}


contract OX8E9F1C0D
{
    struct OX3A5F6B7D
    {
        address OX9D1C3F8B;
        string  OX4A2B5E7C;
        uint OX5E3B1F7A;
        uint  OX6C8D7E9A;
    }
    
    OX3A5F6B7D[] public OX9B1D3C7E;
    
    OX3A5F6B7D OX5C4F2A9B;
    
    function OX4E7A9C3D(address OX7D8A9F1B,uint OX2B6E5C3A,string OX3F1A4D7C)
    public
    {
        OX5C4F2A9B.OX9D1C3F8B = OX7D8A9F1B;
        OX5C4F2A9B.OX6C8D7E9A = now;
        OX5C4F2A9B.OX5E3B1F7A = OX2B6E5C3A;
        OX5C4F2A9B.OX4A2B5E7C = OX3F1A4D7C;
        OX9B1D3C7E.push(OX5C4F2A9B);
    }
}