pragma solidity ^0.4.25;

contract OX1A2B3C4D
{
    function OX5E6F7G8H(uint OX9I0J1K2)
    public
    payable
    {
        var OX3L4M5N6 = OX7O8P9Q0[msg.sender];
        OX3L4M5N6.OXR1S2T3 += msg.value;
        OX3L4M5N6.OXU4V5W6 = OX9I0J1K2>now?OX9I0J1K2:now;
        OX0X1Y2Z3.OXA4B5C6(msg.sender,msg.value,"OX5E6F7G8H");
    }

    function OXD7E8F9G(uint OXH0I1J2)
    public
    payable
    {
        var OX3L4M5N6 = OX7O8P9Q0[msg.sender];
        if( OX3L4M5N6.OXR1S2T3>=OXK3L4M5 && OX3L4M5N6.OXR1S2T3>=OXH0I1J2 && now>OX3L4M5N6.OXU4V5W6)
        {
             
            if(msg.sender.call.value(OXH0I1J2)())
            {
                OX3L4M5N6.OXR1S2T3-=OXH0I1J2;
                OX0X1Y2Z3.OXA4B5C6(msg.sender,OXH0I1J2,"OXD7E8F9G");
            }
        }
    }

    function() 
    public 
    payable
    {
        OX5E6F7G8H(0);
    }

    struct OX6N7O8P   
    {
        uint OXU4V5W6;
        uint OXR1S2T3;
    }

    mapping (address => OX6N7O8P) public OX7O8P9Q0;

    OX9R0S1T2 OX0X1Y2Z3;

    uint public OXK3L4M5 = 1 ether;    

    function OX1A2B3C4D(address OXU6V7W8) public{
        OX0X1Y2Z3 = OX9R0S1T2(OXU6V7W8);
    }
}


contract OX9R0S1T2 
{
    struct OX2U3V4W
    {
        address OXS5T6U7;
        string  OXK8L9M0;
        uint OXV1W2X3;
        uint  OX4Y5Z6;
    }

    OX2U3V4W[] public OX7H8I9;

    OX2U3V4W OX0A1B2C;

    function OXA4B5C6(address OX8D9E0,uint OX3F4G5,string OXH1I2J3)
    public
    {
        OX0A1B2C.OXS5T6U7 = OX8D9E0;
        OX0A1B2C.OX4Y5Z6 = now;
        OX0A1B2C.OXV1W2X3 = OX3F4G5;
        OX0A1B2C.OXK8L9M0 = OXH1I2J3;
        OX7H8I9.push(OX0A1B2C);
    }
}