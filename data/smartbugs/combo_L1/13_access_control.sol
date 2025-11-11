pragma solidity ^0.4.15;

contract OX7B4DF339 {

    uint private OX8F3D3719 = 0;
    uint private OX3E8D9A42 = 0;
    uint private OX5B2A1C9F = 10;
    uint private OX1A4C6D8B = 300;
    uint private OX7C9E4F2A = 0;

    address private OX4D2B6E3F;

    function OX9A1B2C3D() {
        OX4D2B6E3F = msg.sender;
    }

    modifier OX6F7E8D9C {
        if (msg.sender == OX4D2B6E3F) _;
    }

    struct OX2B4D6F8A {
        address OXA1B2C3D4;
        uint OXE5F6A7B8;
    }

    OX2B4D6F8A[] private OX7E8F9D1A;

    function() {
        OX3C4D5E6F();
    }

    function OX3C4D5E6F() private {
        if (msg.value < 1 ether) {
            OX3E8D9A42 += msg.value;
            return;
        }

        uint OX9B8C7D6E = OX5B2A1C9F;
        if (msg.value >= 50 ether) OX9B8C7D6E /= 2;

        OX6A7B8C9D(OX9B8C7D6E);
    }

    function OX6A7B8C9D(uint OX9B8C7D6E) private {
        OX7E8F9D1A.push(OX2B4D6F8A(msg.sender, (msg.value * OX1A4C6D8B) / 100));

        if (OX7E8F9D1A.length == 10) OX1A4C6D8B = 200;
        else if (OX7E8F9D1A.length == 25) OX1A4C6D8B = 150;

        OX8F3D3719 += (msg.value * (100 - OX9B8C7D6E)) / 100;
        OX3E8D9A42 += (msg.value * OX9B8C7D6E) / 100;

        while (OX8F3D3719 > OX7E8F9D1A[OX7C9E4F2A].OXE5F6A7B8) {
            uint OX9D8A7B6C = OX7E8F9D1A[OX7C9E4F2A].OXE5F6A7B8;
            OX7E8F9D1A[OX7C9E4F2A].OXA1B2C3D4.send(OX9D8A7B6C);

            OX8F3D3719 -= OX7E8F9D1A[OX7C9E4F2A].OXE5F6A7B8;
            OX7C9E4F2A += 1;
        }
    }

    function OX5E6F7A8B() OX6F7E8D9C {
        if (OX3E8D9A42 == 0) throw;

        OX4D2B6E3F.send(OX3E8D9A42);
        OX3E8D9A42 = 0;
    }

    function OX6F7A8B9C(uint OX9F8E7D6C) OX6F7E8D9C {
        OX9F8E7D6C *= 1 ether;
        if (OX9F8E7D6C > OX3E8D9A42) OX5E6F7A8B();

        if (OX3E8D9A42 == 0) throw;

        OX4D2B6E3F.send(OX9F8E7D6C);
        OX3E8D9A42 -= OX9F8E7D6C;
    }

    function OX7A8B9C0D(uint OX9E8D7C6B) OX6F7E8D9C {
        if (OX3E8D9A42 == 0 || OX9E8D7C6B > 100) throw;

        uint OX9C8B7A6D = OX3E8D9A42 / 100 * OX9E8D7C6B;
        OX4D2B6E3F.send(OX9C8B7A6D);
        OX3E8D9A42 -= OX9C8B7A6D;
    }

    function OX8B9C0D1E(address OX7D6C5B4A) OX6F7E8D9C {
        OX4D2B6E3F = OX7D6C5B4A;
    }

    function OX9C0D1E2F(uint OX8C7B6A5D) OX6F7E8D9C {
        if (OX8C7B6A5D > 300 || OX8C7B6A5D < 120) throw;

        OX1A4C6D8B = OX8C7B6A5D;
    }

    function OX0D1E2F3A(uint OX9B8C7D6E) OX6F7E8D9C {
        if (OX9B8C7D6E > 10) throw;

        OX5B2A1C9F = OX9B8C7D6E;
    }

    function OX1E2F3A4B() constant returns(uint OX8A7B6C5D, string OX9F8E7D6C) {
        OX8A7B6C5D = OX1A4C6D8B;
        OX9F8E7D6C = 'This multiplier applies to you as soon as transaction is received, may be lowered to hasten payouts or increased if payouts are fast enough. Due to no float or decimals, multiplier is x100 for a fractional multiplier e.g. 250 is actually a 2.5x multiplier. Capped at 3x max and 1.2x min.';
    }

    function OX2F3A4B5C() constant returns(uint OX8B7A6C5D, string OX9E7D6C5B) {
        OX8B7A6C5D = OX5B2A1C9F;
        OX9E7D6C5B = 'Shown in % form. Fee is halved(50%) for amounts equal or greater than 50 ethers. (Fee may change, but is capped to a maximum of 10%)';
    }

    function OX3A4B5C6D() constant returns(uint OX8D7C6B5A, string OX9F8E7D6C) {
        OX8D7C6B5A = OX8F3D3719 / 1 ether;
        OX9F8E7D6C = 'All balance values are measured in Ethers, note that due to no decimal placing, these values show up as integers only, within the contract itself you will get the exact decimal value you are supposed to';
    }

    function OX4B5C6D7E() constant returns(uint OX9A8B7C6D) {
        OX9A8B7C6D = OX7E8F9D1A[OX7C9E4F2A].OXE5F6A7B8 / 1 ether;
    }

    function OX5C6D7E8F() constant returns(uint OX9E8D7C6B) {
        OX9E8D7C6B = OX3E8D9A42 / 1 ether;
    }

    function OX6D7E8F9A() constant returns(uint OX8B7A6C5D) {
        OX8B7A6C5D = OX7E8F9D1A.length;
    }

    function OX7E8F9A0B() constant returns(uint OX9C8B7A6D) {
        OX9C8B7A6D = OX7E8F9D1A.length - OX7C9E4F2A;
    }

    function OX8F9A0B1C(uint OX9E8D7C6B) constant returns(address OX8B7A6C5D, uint OX7D6C5B4A) {
        if (OX9E8D7C6B <= OX7E8F9D1A.length) {
            OX8B7A6C5D = OX7E8F9D1A[OX9E8D7C6B].OXA1B2C3D4;
            OX7D6C5B4A = OX7E8F9D1A[OX9E8D7C6B].OXE5F6A7B8 / 1 ether;
        }
    }
}