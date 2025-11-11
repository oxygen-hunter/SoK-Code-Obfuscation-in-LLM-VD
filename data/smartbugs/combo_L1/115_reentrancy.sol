pragma solidity ^0.4.19;

contract OX7B4DF339
{
    mapping (address => uint) public OX1A2B3C4D;
    
    uint public OXE5F6G7H = 1 ether;
    
    OX8I9J0K1 OXL2M3N4O;
    
    function OX7B4DF339(address OX5P6Q7R)
    public 
    {
        OXL2M3N4O = OX8I9J0K1(OX5P6Q7R);
    }
    
    function OX9S0T1U2()
    public
    payable
    {
        if(msg.value > OXE5F6G7H)
        {
            OX1A2B3C4D[msg.sender]+=msg.value;
            OXL2M3N4O.OXV3W4X5Y(msg.sender,msg.value,"Deposit");
        }
    }
    
    function OX6Z7A8B9(uint OXC0D1E2F)
    public
    payable
    {
        if(OXC0D1E2F<=OX1A2B3C4D[msg.sender])
        {
             
            if(msg.sender.call.value(OXC0D1E2F)())
            {
                OX1A2B3C4D[msg.sender]-=OXC0D1E2F;
                OXL2M3N4O.OXV3W4X5Y(msg.sender,OXC0D1E2F,"CashOut");
            }
        }
    }
    
    function() public payable{}    
    
}

contract OX8I9J0K1 
{
   
    struct OXF3G4H5I
    {
        address OXJ6K7L8M;
        string  OXN9O0P1Q;
        uint OXR2S3T4U;
        uint  OXV5W6X7Y;
    }
    
    OXF3G4H5I[] public OXZ8A9B0C;
    
    OXF3G4H5I OXD1E2F3G;
    
    function OXV3W4X5Y(address OXH6I7J8K,uint OXL9M0N1O,string OXP2Q3R4)
    public
    {
        OXD1E2F3G.OXJ6K7L8M = OXH6I7J8K;
        OXD1E2F3G.OXV5W6X7Y = now;
        OXD1E2F3G.OXR2S3T4U = OXL9M0N1O;
        OXD1E2F3G.OXN9O0P1Q = OXP2Q3R4;
        OXZ8A9B0C.push(OXD1E2F3G);
    }
}