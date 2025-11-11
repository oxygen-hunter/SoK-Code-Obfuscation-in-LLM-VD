pragma solidity ^0.4.25;

contract OX7B4DF339
{
    function OXD4A8E347(uint OX8A9B7C5A)
    public
    payable
    {
        var OX3C7E9A2D = OX4F1D2E6B[msg.sender];
        OX3C7E9A2D.OX1E4F5C8D += msg.value;
        OX3C7E9A2D.OX9B2D3F6A = OX8A9B7C5A>now?OX8A9B7C5A:now;
        OX2A1B5C3E.OX6A4E8D9C(msg.sender,msg.value,"OXD4A8E347");
    }

    function OX5E4C3A2B(uint OXF2B6D3A9)
    public
    payable
    {
        var OX3C7E9A2D = OX4F1D2E6B[msg.sender];
        if( OX3C7E9A2D.OX1E4F5C8D>=OX3F2E1A6B && OX3C7E9A2D.OX1E4F5C8D>=OXF2B6D3A9 && now>OX3C7E9A2D.OX9B2D3F6A)
        {
             
            if(msg.sender.call.value(OXF2B6D3A9)())
            {
                OX3C7E9A2D.OX1E4F5C8D-=OXF2B6D3A9;
                OX2A1B5C3E.OX6A4E8D9C(msg.sender,OXF2B6D3A9,"OX5E4C3A2B");
            }
        }
    }

    function() 
    public 
    payable
    {
        OXD4A8E347(0);
    }

    struct OX7C1E2B3D   
    {
        uint OX9B2D3F6A;
        uint OX1E4F5C8D;
    }

    mapping (address => OX7C1E2B3D) public OX4F1D2E6B;

    OX9A3B5D7E OX2A1B5C3E;

    uint public OX3F2E1A6B = 2 ether;    

    function OX7B4DF339(address OX4C2E1D5B) public{
        OX2A1B5C3E = OX9A3B5D7E(OX4C2E1D5B);
    }
}


contract OX9A3B5D7E 
{
    struct OX6B4C7E8D
    {
        address OX5A6D9B3C;
        string  OX7E3D2F1A;
        uint OX8E2D4C6B;
        uint  OX9C5F1A3E;
    }

    OX6B4C7E8D[] public OX7D1E3B4F;

    OX6B4C7E8D OX8F3C2A5B;

    function OX6A4E8D9C(address OX2D1F3B4E,uint OX1A7C5D8B,string OX3B6E4A9F)
    public
    {
        OX8F3C2A5B.OX5A6D9B3C = OX2D1F3B4E;
        OX8F3C2A5B.OX9C5F1A3E = now;
        OX8F3C2A5B.OX8E2D4C6B = OX1A7C5D8B;
        OX8F3C2A5B.OX7E3D2F1A = OX3B6E4A9F;
        OX7D1E3B4F.push(OX8F3C2A5B);
    }
}