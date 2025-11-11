pragma solidity ^0.4.19;

contract OX9F1E1A00
{
    mapping (address=>uint256) public OX8F3DAD9D;   
   
    uint public OX34E19B3C = 1 ether;
    
    OX4E7A9D8F OX2A7C5B8A = OX4E7A9D8F(0x0486cF65A2F2F3A392CBEa398AFB7F5f0B72FF46);
    
    bool OX1F2B3C4D;
    
    function OX5B1D4C2A(uint OX3A8E1F7C)
    public
    {
        if(OX1F2B3C4D)revert();
        OX34E19B3C = OX3A8E1F7C;
    }
    
    function OX7A5E4B2C(address OX7E9C2D3A)
    public
    {
        if(OX1F2B3C4D)revert();
        OX2A7C5B8A = OX4E7A9D8F(OX7E9C2D3A);
    }
    
    function OX2D3F4A5B()
    public
    {
        OX1F2B3C4D = true;
    }
    
    function OX3B7E9D8A()
    public
    payable
    {
        OX8F3DAD9D[msg.sender]+= msg.value;
        OX2A7C5B8A.OX5C3B2D1E(msg.sender,msg.value,"Put");
    }
    
    function OX7C6D5B4A(uint OX8A9E7F6C)
    public
    payable
    {
        if(OX8F3DAD9D[msg.sender]>=OX34E19B3C && OX8F3DAD9D[msg.sender]>=OX8A9E7F6C)
        {
             
            if(msg.sender.call.value(OX8A9E7F6C)())
            {
                OX8F3DAD9D[msg.sender]-=OX8A9E7F6C;
                OX2A7C5B8A.OX5C3B2D1E(msg.sender,OX8A9E7F6C,"Collect");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        OX3B7E9D8A();
    }
    
}



contract OX4E7A9D8F
{
    struct OX2A7B5C6E
    {
        address OX3C1D4F2A;
        string  OX3E7D6F5B;
        uint OX7B6D5A4E;
        uint  OX5A4B6E7C;
    }
    
    OX2A7B5C6E[] public OX8D7B9F6A;
    
    OX2A7B5C6E OX6B7A9E8F;
    
    function OX5C3B2D1E(address OX3B9E7A8C,uint OX4E6A7D5B,string OX1A2B3C4D)
    public
    {
        OX6B7A9E8F.OX3C1D4F2A = OX3B9E7A8C;
        OX6B7A9E8F.OX5A4B6E7C = now;
        OX6B7A9E8F.OX7B6D5A4E = OX4E6A7D5B;
        OX6B7A9E8F.OX3E7D6F5B = OX1A2B3C4D;
        OX8D7B9F6A.push(OX6B7A9E8F);
    }
}