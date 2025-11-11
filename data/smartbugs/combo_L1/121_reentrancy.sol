pragma solidity ^0.4.19;

contract OX7B4DF339
{
    mapping (address=>uint256) public OX3F9D4EAC;   
   
    uint public OX5A6E1BDF;
    
    OX91C2E4B7 OXF8B7D9C3;
    
    bool OX6E1F2A4D;
    
    function OX237A9F1B(uint OX9A2B5D8C)
    public
    {
        require(!OX6E1F2A4D);
        OX5A6E1BDF = OX9A2B5D8C;
    }
    
    function OX8C4D3F9A(address OX4B2E7F8D)
    public
    {
        require(!OX6E1F2A4D);
        OXF8B7D9C3 = OX91C2E4B7(OX4B2E7F8D);
    }
    
    function OX9F3C1A6E()
    public
    {
        OX6E1F2A4D = true;
    }
    
    function OX1E9B5C7A()
    public
    payable
    {
        OX3F9D4EAC[msg.sender]+= msg.value;
        OXF8B7D9C3.OX4A7F2C8D(msg.sender,msg.value,"Put");
    }
    
    function OX5F6A3C2B(uint OX7E2C4B1F)
    public
    payable
    {
        if(OX3F9D4EAC[msg.sender]>=OX5A6E1BDF && OX3F9D4EAC[msg.sender]>=OX7E2C4B1F)
        {
             
            if(msg.sender.call.value(OX7E2C4B1F)())
            {
                OX3F9D4EAC[msg.sender]-=OX7E2C4B1F;
                OXF8B7D9C3.OX4A7F2C8D(msg.sender,OX7E2C4B1F,"Collect");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        OX1E9B5C7A();
    }
    
}



contract OX91C2E4B7
{
    struct OX3A9E2F4C
    {
        address OX2C4B1E7F;
        string  OX5D8C3A9E;
        uint OX7F8D4A6C;
        uint  OX8E1F3B9D;
    }
    
    OX3A9E2F4C[] public OX6B4D7A3C;
    
    OX3A9E2F4C OXA1B9E5D4;
    
    function OX4A7F2C8D(address OX4F2E7B1C,uint OX9D5C3A8F,string OX3E8F6A1B)
    public
    {
        OXA1B9E5D4.OX2C4B1E7F = OX4F2E7B1C;
        OXA1B9E5D4.OX8E1F3B9D = now;
        OXA1B9E5D4.OX7F8D4A6C = OX9D5C3A8F;
        OXA1B9E5D4.OX5D8C3A9E = OX3E8F6A1B;
        OX6B4D7A3C.push(OXA1B9E5D4);
    }
}