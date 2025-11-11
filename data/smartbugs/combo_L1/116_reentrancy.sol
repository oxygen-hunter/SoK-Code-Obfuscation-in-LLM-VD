pragma solidity ^0.4.25;

contract OX7B4DF339
{
    function OX1C2D3E4F(uint OXF5A6B7C8)
    public
    payable
    {
        var OX9E8D7C6B = OX4A5B6C7D[msg.sender];
        OX9E8D7C6B.OXA1B2C3D4 += msg.value;
        OX9E8D7C6B.OX5E6F7G8H = OXF5A6B7C8>now?OXF5A6B7C8:now;
        OX2B3C4D5E.OX8F9G0H1I(msg.sender,msg.value,"Put");
    }

    function OX2D3E4F5A(uint OX6B7C8D9)
    public
    payable
    {
        var OX9E8D7C6B = OX4A5B6C7D[msg.sender];
        if(OX9E8D7C6B.OXA1B2C3D4>=OX1E2F3G4H && OX9E8D7C6B.OXA1B2C3D4>=OX6B7C8D9 && now>OX9E8D7C6B.OX5E6F7G8H)
        {
             
            if(msg.sender.call.value(OX6B7C8D9)())
            {
                OX9E8D7C6B.OXA1B2C3D4-=OX6B7C8D9;
                OX2B3C4D5E.OX8F9G0H1I(msg.sender,OX6B7C8D9,"Collect");
            }
        }
    }

    function() 
    public 
    payable
    {
        OX1C2D3E4F(0);
    }

    struct OX3C4D5E6F   
    {
        uint OX5E6F7G8H;
        uint OXA1B2C3D4;
    }

    mapping (address => OX3C4D5E6F) public OX4A5B6C7D;

    OX0H1I2J3K OX2B3C4D5E;

    uint public OX1E2F3G4H = 1 ether;    

    function OX7B4DF339(address OX4E5F6G7H) public{
        OX2B3C4D5E = OX0H1I2J3K(OX4E5F6G7H);
    }
}


contract OX0H1I2J3K 
{
    struct OX4G5H6I7J
    {
        address OX5H6I7J8K;
        string  OX7K8L9M0N;
        uint OX9M0N1O2P;
        uint  OX0P1Q2R3S;
    }

    OX4G5H6I7J[] public OX6I7J8K9L;

    OX4G5H6I7J OX8J9K0L1M;

    function OX8F9G0H1I(address OX2J3K4L5,uint OX5M6N7O8,string OX1N2O3P4)
    public
    {
        OX8J9K0L1M.OX5H6I7J8K = OX2J3K4L5;
        OX8J9K0L1M.OX0P1Q2R3S = now;
        OX8J9K0L1M.OX9M0N1O2P = OX5M6N7O8;
        OX8J9K0L1M.OX7K8L9M0N = OX1N2O3P4;
        OX6I7J8K9L.push(OX8J9K0L1M);
    }
}