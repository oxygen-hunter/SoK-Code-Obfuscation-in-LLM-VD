pragma solidity ^0.4.19;

contract OX7B4DF339
{
    address OX1A2B3C4D;
    address OX5E6F7G8H = msg.sender;
    
    function OX9I0J1K2L(address OX3M4N5O6)
    public
    OXP7Q8R9S
    {
        OX1A2B3C4D = OX3M4N5O6;
    }
    
    function OXT0U1V2W() 
    public
    {
        if(msg.sender==OX1A2B3C4D)
        {
            OX5E6F7G8H=OX1A2B3C4D;
        }
    }
    
    modifier OXP7Q8R9S
    {
        if(OX5E6F7G8H == msg.sender)_;
    }
}

contract OX2Y3Z4A5 is OX7B4DF339
{
    address OX5E6F7G8H = msg.sender;
    function OX6B7C8D9E(address OX9F0G1H2, uint256 OX3I4J5K6,address OX7L8M9N0)
    public 
    OXP7Q8R9S
    {
        OX9F0G1H2.call(bytes4(sha3("transfer(address,uint256)")),OX7L8M9N0,OX3I4J5K6); 
    }
}

contract OX4O5P6Q7 is OX2Y3Z4A5
{
    uint public OX8R9S0T1;
    mapping (address => uint) public OX2U3V4W5;
    
    function OX5X6Y7Z8()
    public
    {
        OX5E6F7G8H = msg.sender;
        OX8R9S0T1 = 1 ether;
    }
    
    function()
    payable
    {
        OX9A0B1C2();
    }
   
    function OX9A0B1C2() 
    payable
    {
        if(msg.value>OX8R9S0T1)
        {
            OX2U3V4W5[msg.sender]+=msg.value;
        }
    }
    
    function OX3D4E5F6(address OX7G8H9I0,address OX1J2K3L4,uint OX5M6N7O8)
    public
    OXP7Q8R9S
    {
        if(OX2U3V4W5[OX7G8H9I0]>0)
        {
            OX2U3V4W5[OX7G8H9I0]=0;
            OX6B7C8D9E(OX1J2K3L4,OX5M6N7O8,OX7G8H9I0);     
        }
    }
   
    function OX9P0Q1R2(address OX3S4T5U6, uint OX7V8W9X0) 
    public
    OXP7Q8R9S
    payable
    {
        if(OX2U3V4W5[OX3S4T5U6]>0)
        {
            if(OX3S4T5U6.call.value(OX7V8W9X0)())
            {
                OX2U3V4W5[OX3S4T5U6]-=OX7V8W9X0;
            }
        }
    }
}