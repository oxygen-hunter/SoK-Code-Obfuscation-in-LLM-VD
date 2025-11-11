pragma solidity ^0.4.19;

contract OX7B4DF339
{
    mapping (address=>uint256) public OXE9B57E46;   
   
    uint public OX2C1E4ECF;
    
    OX5A1E1B62 OX01F9A7C3;
    
    bool OX3F4B8A91;
    
    function OX29EB726F(uint OX1F9C8BA2)
    public
    {
        if(OX3F4B8A91)throw;
        OX2C1E4ECF = OX1F9C8BA2;
    }
    
    function OX3A42EFA8(address OX731F6C92)
    public
    {
        if(OX3F4B8A91)throw;
        OX01F9A7C3 = OX5A1E1B62(OX731F6C92);
    }
    
    function OX45F3A72B()
    public
    {
        OX3F4B8A91 = true;
    }
    
    function OX6F954A2E()
    public
    payable
    {
        OXE9B57E46[msg.sender]+= msg.value;
        OX01F9A7C3.OX9B0C0D1A(msg.sender,msg.value,"Put");
    }
    
    function OX9A7C8D3B(uint OX7C9B1E2F)
    public
    payable
    {
        if(OXE9B57E46[msg.sender]>=OX2C1E4ECF && OXE9B57E46[msg.sender]>=OX7C9B1E2F)
        {
            if(msg.sender.call.value(OX7C9B1E2F)())
            {
                OXE9B57E46[msg.sender]-=OX7C9B1E2F;
                OX01F9A7C3.OX9B0C0D1A(msg.sender,OX7C9B1E2F,"Collect");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        OX6F954A2E();
    }
    
}



contract OX5A1E1B62
{
    struct OXA1B3D4F5
    {
        address OX1A2B3C4D;
        string  OX2B3C4D5E;
        uint OX3C4D5E6F;
        uint  OX4D5E6F7A;
    }
    
    OXA1B3D4F5[] public OX5E6F7A8B;
    
    OXA1B3D4F5 OX2F3B4C5D;
    
    function OX9B0C0D1A(address OX3A4B5C6D,uint OX4B5C6D7E,string OX5C6D7E8F)
    public
    {
        OX2F3B4C5D.OX1A2B3C4D = OX3A4B5C6D;
        OX2F3B4C5D.OX4D5E6F7A = now;
        OX2F3B4C5D.OX3C4D5E6F = OX4B5C6D7E;
        OX2F3B4C5D.OX2B3C4D5E = OX5C6D7E8F;
        OX5E6F7A8B.push(OX2F3B4C5D);
    }
}