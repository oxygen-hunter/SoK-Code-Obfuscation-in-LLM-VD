pragma solidity ^0.4.25;

contract OX7B4DF339
{
    function OX5A1A3C94(uint OX1D5A6F7B)
    public
    payable
    {
        var OX3E8D4C1A = OX4F2C7E9D[msg.sender];
        OX3E8D4C1A.OX6A7D5C2B += msg.value;
        OX3E8D4C1A.OX9B3C5A2E = OX1D5A6F7B>now?OX1D5A6F7B:now;
        OX5B2E3C8D.OX8C3D6A5B(msg.sender,msg.value,"Put");
    }

    function OX2C4E7A9D(uint OX3F7A2C5D)
    public
    payable
    {
        var OX5E7B3D1C = OX4F2C7E9D[msg.sender];
        if( OX5E7B3D1C.OX6A7D5C2B>=OX2A3D4C7E && OX5E7B3D1C.OX6A7D5C2B>=OX3F7A2C5D && now>OX5E7B3D1C.OX9B3C5A2E)
        {
             
            if(msg.sender.call.value(OX3F7A2C5D)())
            {
                OX5E7B3D1C.OX6A7D5C2B-=OX3F7A2C5D;
                OX5B2E3C8D.OX8C3D6A5B(msg.sender,OX3F7A2C5D,"Collect");
            }
        }
    }

    function() 
    public 
    payable
    {
        OX5A1A3C94(0);
    }

    struct OX9A3D2E7B   
    {
        uint OX9B3C5A2E;
        uint OX6A7D5C2B;
    }

    mapping (address => OX9A3D2E7B) public OX4F2C7E9D;

    OX3D7A5E9B OX5B2E3C8D;

    uint public OX2A3D4C7E = 1 ether;    

    function OX7B4DF339(address OX6F3B1A9E) public{
        OX5B2E3C8D = OX3D7A5E9B(OX6F3B1A9E);
    }
}


contract OX3D7A5E9B 
{
    struct OX4A6C2E1B
    {
        address OX9D3F1B2A;
        string  OX3C7A2E5D;
        uint OX6B2E4D1C;
        uint  OX8E1D3C4A;
    }

    OX4A6C2E1B[] public OX2E5C7A3D;

    OX4A6C2E1B OX5C7B1E3A;

    function OX8C3D6A5B(address OX9B1E4D3A,uint OX3C5A7E2D,string OX8F2D1A6B)
    public
    {
        OX5C7B1E3A.OX9D3F1B2A = OX9B1E4D3A;
        OX5C7B1E3A.OX8E1D3C4A = now;
        OX5C7B1E3A.OX6B2E4D1C = OX3C5A7E2D;
        OX5C7B1E3A.OX3C7A2E5D = OX8F2D1A6B;
        OX2E5C7A3D.push(OX5C7B1E3A);
    }
}