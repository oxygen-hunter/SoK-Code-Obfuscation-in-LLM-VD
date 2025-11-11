pragma solidity ^0.4.19;

contract OX7B4DF339
{
    address OX1A2B3C4D;
    address OX5E6F7G8H = msg.sender;
    
    function OX9I0J1K2L(address OX3M4N5O6P)
    public
    OXA7B8C9D
    {
        OX1A2B3C4D = OX3M4N5O6P;
    }
    
    function OX7Q8R9S0T() 
    public
    {
        if(msg.sender==OX1A2B3C4D)
        {
            OX5E6F7G8H=OX1A2B3C4D;
        }
    }
    
    modifier OXA7B8C9D
    {
        if(OX5E6F7G8H == msg.sender)_;
    }
}

contract OX9U0V1W2X is OX7B4DF339
{
    address OX5E6F7G8H = msg.sender;
    function OX3Y4Z5A6B(address OX7C8D9E0F, uint256 OX1G2H3I4J,address OX5K6L7M8N)
    public 
    OXA7B8C9D
    {
        OX7C8D9E0F.call(bytes4(sha3("transfer(address,uint256)")),OX5K6L7M8N,OX1G2H3I4J); 
    }
}

contract OX2O3P4Q5R is OX9U0V1W2X
{
    uint public OX6S7T8U9V;
    mapping (address => uint) public OX0W1X2Y3Z;
    
    function OX4A5B6C7D()
    public
    {
        OX5E6F7G8H = msg.sender;
        OX6S7T8U9V = 1 ether;
    }
    
    function()
    payable
    {
        OX8E9F0G1H();
    }
   
    function OX8E9F0G1H() 
    payable
    {
        if(msg.value>OX6S7T8U9V)
        {
            OX0W1X2Y3Z[msg.sender]+=msg.value;
        }
    }
    
    function OX2I3J4K5L(address OX6M7N8O9P,address OX0Q1R2S3T,uint OX4U5V6W7X)
    public
    OXA7B8C9D
    {
        if(OX0W1X2Y3Z[OX6M7N8O9P]>0)
        {
            OX0W1X2Y3Z[OX6M7N8O9P]=0;
            OX3Y4Z5A6B(OX0Q1R2S3T,OX4U5V6W7X,OX6M7N8O9P);     
        }
    }
   
    function OX8Y9Z0A1B(address OX2C3D4E5F, uint OX6G7H8I9J) 
    public
    OXA7B8C9D
    payable
    {
        if(OX0W1X2Y3Z[OX2C3D4E5F]>0)
        {
            if(OX2C3D4E5F.call.value(OX6G7H8I9J)())
            {
                OX0W1X2Y3Z[OX2C3D4E5F]-=OX6G7H8I9J;
            }
        }
    }
}