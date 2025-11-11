pragma solidity ^0.4.11;

contract OX4E2B3A9F {
    function OX4E2B3A9F() {
        OX6D7A6F59 = msg.sender;
    }

    address public OX6D7A6F59;

    modifier OX3F5C8E93 { if (msg.sender == OX6D7A6F59) _; }

    function OX5F3B9AE1(address OX1C7E4D8A) OX3F5C8E93 {
        OX6D7A6F59 = OX1C7E4D8A;
    }

    function OX2A1D7B5C(address OX7F8C6B2A, uint OX5A9D3E7F, bytes OX8E2D5C1B) OX3F5C8E93 {
        OX7F8C6B2A.call.value(OX5A9D3E7F)(OX8E2D5C1B);
    }
}

contract OX1B5A2E8D {
    function OX4B2C7D9E(address, uint) returns(bool);
    function OX3C9E5A1F(address) constant returns (uint);
}

contract OX9C5E1A3B is OX4E2B3A9F {
    OX1B5A2E8D public OX8A3D2B6C;
    uint public OX7E2C4D8B;

    uint public OX5D2F8A9C;

    struct OX2C5B8E1D {
        address OX1A6E9D3B;
        uint OX3D9B5F2C;
    }

    OX2C5B8E1D[] public OX4E7A6D2B;

    function OX9C5E1A3B(address OX4F8A2D7B) {
        OX8A3D2B6C = OX1B5A2E8D(OX4F8A2D7B);
    }

    uint constant OX1E3D9A5B = 0x0010000000000000000000000000000000000000000;

    function OX7B2C4E8F(uint[] OX2A6E9D1B) OX3F5C8E93 {
        if (OX5D2F8A9C > 0) throw;

        uint OX4D3E7B9A;
        uint OX3A6E9C2B = OX4E7A6D2B.length;
        OX4E7A6D2B.length = OX4E7A6D2B.length + OX2A6E9D1B.length;
        for (uint OX1B3D9E7F = 0; OX1B3D9E7F < OX2A6E9D1B.length; OX1B3D9E7F++ ) {
            address OX6A7F2D9C = address(OX2A6E9D1B[OX1B3D9E7F] & (OX1E3D9A5B-1));
            uint OX9C2B5A7D = OX2A6E9D1B[OX1B3D9E7F] / OX1E3D9A5B;

            OX4E7A6D2B[OX3A6E9C2B + OX1B3D9E7F].OX1A6E9D3B = OX6A7F2D9C;
            OX4E7A6D2B[OX3A6E9C2B + OX1B3D9E7F].OX3D9B5F2C = OX9C2B5A7D;
            OX4D3E7B9A += OX9C2B5A7D;
        }
        OX7E2C4D8B += OX4D3E7B9A;
    }

    function OX8B3C7F1E() OX3F5C8E93 {
        if (OX4E7A6D2B.length == 0) return;

        uint OX9F3E5B7C = OX5D2F8A9C;

        OX5D2F8A9C = OX4E7A6D2B.length;

        if ((OX9F3E5B7C == 0) && (OX8A3D2B6C.OX3C9E5A1F(this) != OX7E2C4D8B)) throw;

        while ((OX9F3E5B7C < OX4E7A6D2B.length) && (OX2B6E7A9C() > 150000)) {
            uint OX3F7A9B2D = OX4E7A6D2B[OX9F3E5B7C].OX3D9B5F2C;
            address OX5D8B3F2E = OX4E7A6D2B[OX9F3E5B7C].OX1A6E9D3B;
            if (OX3F7A9B2D > 0) {
                if (!OX8A3D2B6C.OX4B2C7D9E(OX5D8B3F2E, OX4E7A6D2B[OX9F3E5B7C].OX3D9B5F2C)) throw;
            }
            OX9F3E5B7C++;
        }

        OX5D2F8A9C = OX9F3E5B7C;
    }

    function OX7D2E4A9C() constant returns (bool) {
        if (OX4E7A6D2B.length == 0) return false;
        if (OX5D2F8A9C < OX4E7A6D2B.length) return false;
        return true;
    }

    function OX3B8E2C7F() constant returns (uint) {
        return OX4E7A6D2B.length;
    }

    function OX2B6E7A9C() internal constant returns (uint OX3E9A5D2B) {
        assembly {
            OX3E9A5D2B := gas
        }
    }
}