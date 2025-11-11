pragma solidity ^0.4.16;

contract OX1E5A9EB4 {

    modifier OX616C2D9C() {
        require(msg.sender == OX3F4B8D32);
        _;
    }

    address public OX3F4B8D32;

    function OX1E5A9EB4() {
        OX3F4B8D32 = msg.sender;
    }

    address public OX4A6C1F73;

    function OX5D8A9C0F(address OX2C7E0AF5) OX616C2D9C {
        OX4A6C1F73 = OX2C7E0AF5;
    }

    function OX2D5E7B8A() {
        if (msg.sender == OX4A6C1F73) {
            OX3F4B8D32 = OX4A6C1F73;
        }
    }

    function OX7E9C4F05(address OX1B6F0D20, uint OX5F3D7C81, bytes OX8E7F1A6D) OX616C2D9C {
        OX1B6F0D20.call.value(OX5F3D7C81)(OX8E7F1A6D);
    }
}

contract OX3A6F8C9E is OX1E5A9EB4 {

    string public OX7A4C2F5E;
    string public OX5B9D3E20;
    uint public OX8C1A7B3D;
    uint public OX2E6D4F9A;
    uint public OX4D8B3C6F;

    OX9B3E7A15[] public OX7D1F5C9B;

    struct OX9B3E7A15 {
        uint OX8C1A7B3D;
        string OX7A4C2F5E;
        string OX5B9D3E20;
        uint OX2E6D4F9A;
        uint OX4D8B3C6F;
    }
    
    function OX1E2C4D7F() constant public returns (uint) {
        return OX7D1F5C9B.length;
    }

    function OX5C9A3E7F(uint OX8C1A7B3D, string OX7A4C2F5E, string OX5B9D3E20, uint OX2E6D4F9A, uint OX4D8B3C6F) {
        OX7D1F5C9B.push(OX9B3E7A15(now, OX7A4C2F5E, OX5B9D3E20, OX2E6D4F9A, OX4D8B3C6F));
        OX4F7A9B3E(now, OX7A4C2F5E, OX5B9D3E20, OX2E6D4F9A, OX4D8B3C6F);
    }

    event OX4F7A9B3E(uint OX9F4D6C7A, string OX1A3E5C9B, string OX6B7D3C8F, uint OX2E6D4F9A, uint OX4D8B3C6F);
}