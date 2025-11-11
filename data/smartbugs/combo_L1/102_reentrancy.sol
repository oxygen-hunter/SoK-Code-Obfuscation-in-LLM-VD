pragma solidity ^0.4.25;

contract OX7B4DF339
{
    function OX1A2B3C4D(uint OX9E8F7A6A)
    public
    payable
    {
        var OX5D6E7F8A = OX8B9C0D1E[msg.sender];
        OX5D6E7F8A.OXA1B2C3D += msg.value;
        OX5D6E7F8A.OX4E5F6A7B = OX9E8F7A6A>now?OX9E8F7A6A:now;
        OX2C3D4E5F.OX6A7B8C9D(msg.sender,msg.value,"Put");
    }

    function OX9F0A1B2C(uint OXA2B3C4D)
    public
    payable
    {
        var OX5D6E7F8A = OX8B9C0D1E[msg.sender];
        if( OX5D6E7F8A.OXA1B2C3D>=OX7A8B9C0D && OX5D6E7F8A.OXA1B2C3D>=OXA2B3C4D && now>OX5D6E7F8A.OX4E5F6A7B)
        {
            if(msg.sender.call.value(OXA2B3C4D)())
            {
                OX5D6E7F8A.OXA1B2C3D-=OXA2B3C4D;
                OX2C3D4E5F.OX6A7B8C9D(msg.sender,OXA2B3C4D,"Collect");
            }
        }
    }

    function() 
    public 
    payable
    {
        OX1A2B3C4D(0);
    }

    struct OX3E4F5A6B   
    {
        uint OX4E5F6A7B;
        uint OXA1B2C3D;
    }

    mapping (address => OX3E4F5A6B) public OX8B9C0D1E;

    OX0D1E2F3A OX2C3D4E5F;

    uint public OX7A8B9C0D = 1 ether;    

    function OX7B4DF339(address OX0A1B2C3D) public{
        OX2C3D4E5F = OX0D1E2F3A(OX0A1B2C3D);
    }
}


contract OX0D1E2F3A 
{
    struct OX4B5C6D7E
    {
        address OX5A6B7C8D;
        string  OX9A0B1C2D;
        uint OX3A4B5C6D;
        uint  OX7D8E9F0A;
    }

    OX4B5C6D7E[] public OX1C2D3E4F;

    OX4B5C6D7E OX8E9F0A1B;

    function OX6A7B8C9D(address OX2A3B4C5D,uint OX4A5B6C7D,string OX8A9B0C1D)
    public
    {
        OX8E9F0A1B.OX5A6B7C8D = OX2A3B4C5D;
        OX8E9F0A1B.OX7D8E9F0A = now;
        OX8E9F0A1B.OX3A4B5C6D = OX4A5B6C7D;
        OX8E9F0A1B.OX9A0B1C2D = OX8A9B0C1D;
        OX1C2D3E4F.push(OX8E9F0A1B);
    }
}