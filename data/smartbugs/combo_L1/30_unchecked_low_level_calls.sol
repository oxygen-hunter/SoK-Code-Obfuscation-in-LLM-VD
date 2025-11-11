pragma solidity ^0.4.0;

contract OX7B4DF339 {

    struct OX9F1E6A4C {
        address OX5D9E9C88;
        string OX0A58D9C2;
        uint OX9A0BCDE5;
        uint OXA4C5EC9D;
    }

    address OX1A0EDB3F;

    modifier OX9A8C3B12 { if (msg.sender == OX1A0EDB3F) _; }

    uint constant OX4F9A2B3D = 100 finney;

    uint constant OX8B6A1D3C = 3;
    uint constant OX7C3B9F8D = 2;

    uint constant OX3B5D1F7E = 1;
    uint constant OX2F6C4D8B = 100;

    uint public OX6E2A9D0C;

    OX9F1E6A4C public OX0F8D4C6E;

    OX9F1E6A4C[] public OX8C4A1F5E;

    function OX7B4DF339() {
        OX1A0EDB3F = msg.sender;
        OX6E2A9D0C = OX4F9A2B3D;
        OX0F8D4C6E = OX9F1E6A4C(
            OX1A0EDB3F,
            "[Vacant]",
            0,
            block.timestamp
        );
    }

    function OX3A9E5B6D() constant returns (uint OX2F5C7A1E) {
        return OX8C4A1F5E.length;
    }

    event OX7A9E2B5C(
        address OX9C4D7F3B,
        string OX1E5F9C3A,
        uint OX0D8A4F6E
    );

    function() {
        OX5B3A9F7D(string(msg.data));
    }

    function OX5B3A9F7D(string OX0A9C5D8E) {

        uint OX8D3A6F5C = msg.value;

        if (OX8D3A6F5C < OX6E2A9D0C) {
            msg.sender.send(OX8D3A6F5C);
            return;
        }

        if (OX8D3A6F5C > OX6E2A9D0C) {
            uint OX1A9E4F7C = OX8D3A6F5C - OX6E2A9D0C;
            msg.sender.send(OX1A9E4F7C);
            OX8D3A6F5C = OX8D3A6F5C - OX1A9E4F7C;
        }

        uint OX3D5C7A9B = (OX8D3A6F5C * OX3B5D1F7E) / OX2F6C4D8B;

        uint OX4A6E9C3D = OX8D3A6F5C - OX3D5C7A9B;

        if (OX0F8D4C6E.OX5D9E9C88 != OX1A0EDB3F) {
            OX0F8D4C6E.OX5D9E9C88.send(OX4A6E9C3D);
        } else {
        }

        OX8C4A1F5E.push(OX0F8D4C6E);
        OX0F8D4C6E = OX9F1E6A4C(
            msg.sender,
            OX0A9C5D8E,
            OX8D3A6F5C,
            block.timestamp
        );

        uint OX5C8A3F1D = OX6E2A9D0C * OX8B6A1D3C / OX7C3B9F8D;
        if (OX5C8A3F1D < 10 finney) {
            OX6E2A9D0C = OX5C8A3F1D;
        } else if (OX5C8A3F1D < 100 finney) {
            OX6E2A9D0C = 100 szabo * (OX5C8A3F1D / 100 szabo);
        } else if (OX5C8A3F1D < 1 ether) {
            OX6E2A9D0C = 1 finney * (OX5C8A3F1D / 1 finney);
        } else if (OX5C8A3F1D < 10 ether) {
            OX6E2A9D0C = 10 finney * (OX5C8A3F1D / 10 finney);
        } else if (OX5C8A3F1D < 100 ether) {
            OX6E2A9D0C = 100 finney * (OX5C8A3F1D / 100 finney);
        } else if (OX5C8A3F1D < 1000 ether) {
            OX6E2A9D0C = 1 ether * (OX5C8A3F1D / 1 ether);
        } else if (OX5C8A3F1D < 10000 ether) {
            OX6E2A9D0C = 10 ether * (OX5C8A3F1D / 10 ether);
        } else {
            OX6E2A9D0C = OX5C8A3F1D;
        }

        OX7A9E2B5C(OX0F8D4C6E.OX5D9E9C88, OX0F8D4C6E.OX0A58D9C2, OX6E2A9D0C);
    }

    function OX2B8E9A1C(uint OX5D7A3F6B) OX9A8C3B12 {
        OX1A0EDB3F.send(OX5D7A3F6B);
    }

    function OX4A9C2D8E(address OX7F3B1A6D) OX9A8C3B12 {
        OX1A0EDB3F = OX7F3B1A6D;
    }

}