pragma solidity ^0.4.16;

contract OX7B4DF339 {

    modifier OXB1E4E74B() {
        require(msg.sender == OX6F4FCE8F);
        _;
    }

    address public OX6F4FCE8F;

    function OX7B4DF339() {
        OX6F4FCE8F = msg.sender;
    }

    address public OX4E6F1F4D;

    function OX8A4B3E12(address OX7D3E8B4F) OXB1E4E74B {
        OX4E6F1F4D = OX7D3E8B4F;
    }

    function OXB2D4E3A9() {
        if (msg.sender == OX4E6F1F4D) {
            OX6F4FCE8F = OX4E6F1F4D;
        }
    }

    function OXF3B5C4D8(address OX5A6D9E3C, uint OX9E8C7BFD, bytes OX3B2E1A7F) OXB1E4E74B {
        OX5A6D9E3C.call.value(OX9E8C7BFD)(OX3B2E1A7F);
    }
}

contract OX9F5D3A7B is OX7B4DF339 {

    string public OX1A2B3C4D;
    string public OX4C3B2A1D;
    uint public OX5C4D3B2A;
    string public OX3C2B1A4D;
    string public OX2D1C3B4A;

    OX7A6B5C4D[] public OX9D8C7B6A;
    OX6A5B4C3D[] public OX8B7A6D5C;

    struct OX7A6B5C4D {
        uint OX6D5C4B3A;
        string OX5B4A3C2D;
        string OX4A3B2C1D;
        string OX3B2C1A4D;
    }

    struct OX6A5B4C3D {
        uint OX2C1B3A4D;
        string OX1A4B3C2D;
        string OX4B3A2C1D;
        string OX3A2B1C4D;
        uint OX1C4B3A2D;
    }

    modifier OX2B1A4D3C {
        require(sha3(OX3C2B1A4D) == sha3("Married"));
        _;
    }

    function OX9F5D3A7B(address OX7F8E9D6C) {
        OX6F4FCE8F = OX7F8E9D6C;
    }

    function OX7C6D5E4F() constant public returns (uint) {
        return OX9D8C7B6A.length;
    }

    function OX8E7F6D5C() constant public returns (uint) {
        return OX8B7A6D5C.length;
    }

    function OX5E4F3D6C(
        string OX6E5F4D3C,
        string OX3D2C1B4A,
        string OX2A1B4C3D,
        string OX1B4A3C2D) OXB1E4E74B
    {
        require(OX9D8C7B6A.length == 0);
        OX1A2B3C4D = OX6E5F4D3C;
        OX4C3B2A1D = OX3D2C1B4A;
        OX5C4D3B2A = now;
        OX2D1C3B4A = OX2A1B4C3D;
        OX3C2B1A4D = "Married";
        OX9D8C7B6A.push(OX7A6B5C4D(now, "Marriage", OX2D1C3B4A, OX1B4A3C2D));
        OXA1B2C3D4("Marrigage", OX2D1C3B4A, OX1B4A3C2D);
    }

    function OX4D3C2B1A(string OX2C3D1A4B, string OX1D2A3B4C) OXB1E4E74B {
        OX3C2B1A4D = OX2C3D1A4B;
        OX3E2D1C4B("Changed Status", OX2C3D1A4B, OX1D2A3B4C);
    }

    function OX3E2D1C4B(string OX1C4D3B2A, string OX2B3A1D4C, string OX3A4B2C1D) OXB1E4E74B OX2B1A4D3C {
        OX9D8C7B6A.push(OX7A6B5C4D(now, OX1C4D3B2A, OX2B3A1D4C, OX3A4B2C1D));
        OXA1B2C3D4(OX1C4D3B2A, OX2B3A1D4C, OX3A4B2C1D);
    }

    function OX2F1E3D4C(string OX4C2D1A3B, string OX3B4A2C1D, string OX1D2C3B4A) payable OX2B1A4D3C {
        if (msg.value > 0) {
            OX6F4FCE8F.transfer(this.balance);
        }
        OX8B7A6D5C.push(OX6A5B4C3D(now, OX4C2D1A3B, OX3B4A2C1D, OX1D2C3B4A, msg.value));
        OX1A3B2C4D(OX4C2D1A3B, OX3B4A2C1D, OX1D2C3B4A, msg.value);
    }

    event OXA1B2C3D4(string OX4D3A2B1C, string OX2C3B1D4A, string OX3C4B2A1D);
    event OX1A3B2C4D(string OX3A2B4C1D, string OX4B3C2A1D, string OX2D3C1B4A, uint OX1C3B4A2D);
}