pragma solidity ^0.4.0;

contract OX7B4DF339 {

    address private OX9F1A7BCA;

    uint private OX2D91F9C3 = 0;
    uint private OX3A9C5E1D = 5;
    uint private OX4E7618A2 = 125;

    mapping (address => OX1A2B3C4D) private OXC5B3A8F2;
    OX5F9A8B3C[] private OX6D7E8F1A;
    uint[] private OX7C8B9D0E;

    function OX7B4DF339() {
        OX9F1A7BCA = msg.sender;
    }

    modifier OX8E4F2C1B { if (msg.sender == OX9F1A7BCA) _; }

    struct OX1A2B3C4D {
        address OX12345678;
        uint OX9ABCDE0F;
        uint OX4B3C2D1E;
    }

    struct OX5F9A8B3C {
        address OX5A6B7C8D;
        uint OX6A7B8C9D;
        uint OX7A8B9C0D;
        bool OX0A1B2C3D;
    }

    function() {
        OXF1E2D3C4();
    }

    function OXF1E2D3C4() private {

        if (msg.value < 1 ether) {
            msg.sender.send(msg.value);
            return;
        }

        OXD4C3B2A1();
    }

    function OXD4C3B2A1() private {

        uint OX8C7B6A5D = 1 ether;

        if (msg.value > 1 ether) {

            msg.sender.send(msg.value - 1 ether);
            OX8C7B6A5D = 1 ether;
        }

        if (OXC5B3A8F2[msg.sender].OX12345678 == address(0)) {
            OXC5B3A8F2[msg.sender].OX12345678 = msg.sender;
            OXC5B3A8F2[msg.sender].OX9ABCDE0F = 0;
            OXC5B3A8F2[msg.sender].OX4B3C2D1E = 0;
        }

        OX6D7E8F1A.push(OX5F9A8B3C(msg.sender, OX8C7B6A5D, (OX8C7B6A5D * (OX4E7618A2) / 100), false));
        OXC5B3A8F2[msg.sender].OX9ABCDE0F++;
        OX7C8B9D0E.push(OX6D7E8F1A.length -1);

        OX2D91F9C3 += (OX8C7B6A5D * (100 - OX3A9C5E1D)) / 100;

        uint OX7E9A6C5D = OX7C8B9D0E.length > 1 ? OX9E8F1C2B(OX7C8B9D0E.length) : 0;
        OX5F9A8B3C OX8D7E6F2C = OX6D7E8F1A[OX7C8B9D0E[OX7E9A6C5D]];

        if (OX2D91F9C3 > OX8D7E6F2C.OX7A8B9C0D) {

            uint OX1F2E3D4C = OX8D7E6F2C.OX7A8B9C0D;

            OX8D7E6F2C.OX5A6B7C8D.send(OX1F2E3D4C);
            OX8D7E6F2C.OX0A1B2C3D = true;
            OXC5B3A8F2[OX8D7E6F2C.OX5A6B7C8D].OX4B3C2D1E++;

            OX2D91F9C3 -= OX1F2E3D4C;

            if (OX7E9A6C5D < OX7C8B9D0E.length - 1)
                OX7C8B9D0E[OX7E9A6C5D] = OX7C8B9D0E[OX7C8B9D0E.length - 1];

            OX7C8B9D0E.length--;

        }

        uint OX9C8B7A6D = this.balance - OX2D91F9C3;
        if (OX9C8B7A6D > 0) {
            OX9F1A7BCA.send(OX9C8B7A6D);
        }

    }

    uint256 constant private OX3E4D2C1F =  1157920892373161954235709850086879078532699846656405640394575840079131296399;

    function OX9E8F1C2B(uint OX2B3C4D5A) constant private returns (uint256 OX1A2B3C4D){
        uint256 OX5A6B7C8D = OX3E4D2C1F * 100 / OX2B3C4D5A;
        uint256 OX7A8B9C0D = block.number - 1;
        uint256 OX9C0D1E2A = uint256(block.blockhash(OX7A8B9C0D));

        return uint256((uint256(OX9C0D1E2A) / OX5A6B7C8D)) % OX2B3C4D5A;
    }

    function OXA1B2C3D4(address OXC4D5E6F7) OX8E4F2C1B {
        OX9F1A7BCA = OXC4D5E6F7;
    }

    function OXE1F2D3C4(uint OX4E5F6A7B) OX8E4F2C1B {
        if (OX4E5F6A7B < 110 || OX4E5F6A7B > 150) throw;

        OX4E7618A2 = OX4E5F6A7B;
    }

    function OXF2C3B4A5(uint OX5F6A7B8C) OX8E4F2C1B {
        if (OX3A9C5E1D > 5)
            throw;
        OX3A9C5E1D = OX5F6A7B8C;
    }

    function OXG2B3C4D5() constant returns (uint OX6A7B8C9D, string OX0D1E2A3B) {
        OX6A7B8C9D = OX4E7618A2;
        OX0D1E2A3B = 'The current multiplier applied to all deposits. Min 110%, max 150%.';
    }

    function OXF3B4C5D6() constant returns (uint OX7A8B9C0D, string OX1D2E3A4B) {
        OX7A8B9C0D = OX3A9C5E1D;
        OX1D2E3A4B = 'The fee percentage applied to all deposits. It can change to speed payouts (max 5%).';
    }

    function OXH3C4D5E6() constant returns (uint OX8B9C0D1A, string OX2E3A4B5C) {
        OX8B9C0D1A = OX6D7E8F1A.length;
        OX2E3A4B5C = 'The number of deposits.';
    }

    function OXI4D5E6F7(address OX3F4A5B6C) constant returns (uint OX9C0D1E2A, uint OX4A5B6C7D, string OX5E6A7B8C) {
        if (OXC5B3A8F2[OX3F4A5B6C].OX12345678 != address(0x0)) {
            OX9C0D1E2A = OXC5B3A8F2[OX3F4A5B6C].OX9ABCDE0F;
            OX4A5B6C7D = OXC5B3A8F2[OX3F4A5B6C].OX4B3C2D1E;
            OX5E6A7B8C = 'Users stats: total deposits, payouts received.';
        }
    }

    function OXJ5E6F7A8(uint OX6B7C8D9E) constant returns (address OX5A6B7C8D, uint OX7A8B9C0D, bool OX0B1C2D3E, string OX9F0A1B2C) {
        if (OX6B7C8D9E < OX6D7E8F1A.length) {
            OX5A6B7C8D = OX6D7E8F1A[OX6B7C8D9E].OX5A6B7C8D;
            OX7A8B9C0D = OX6D7E8F1A[OX6B7C8D9E].OX7A8B9C0D / 1 finney;
            OX0B1C2D3E = OX6D7E8F1A[OX6B7C8D9E].OX0A1B2C3D;
            OX9F0A1B2C = 'Entry info: user address, expected payout in Finneys, payout status.';
        }
    }

}