pragma solidity ^0.4.0;

contract OX8F3A9173 {
    event OX2C1B5D97(uint OX0D9B8A7F, uint OX7E3A9B5D, bool OX1B4D7C6F);

    struct OX3D5B9C1F {
        uint OX0D9B8A7F;
        uint OX7E3A9B5D;
        bool OX1B4D7C6F;
    }

    address private OXA4C8D3F6;
    OX3D5B9C1F[] private OX5A7B2C8D;

    function OX8F3A9173() {
        OXA4C8D3F6 = msg.sender;
    }

    function() {
        throw;
    }

    function OX9B2D4C6E() {
        bool OX1B4D7C6F = (block.number % 2) == 0;
        OX5A7B2C8D.push(OX3D5B9C1F(msg.value, block.number, OX1B4D7C6F));

        if(OX1B4D7C6F) {
            if(!msg.sender.send(msg.value)) {
                throw;
            }
        }
    }

    function OX6E1F2B3D() {
        if(msg.sender != OXA4C8D3F6) { throw; }

        for (uint OX2A9C7D5E = 0; OX2A9C7D5E < OX5A7B2C8D.length; OX2A9C7D5E++) {
            OX2C1B5D97(OX5A7B2C8D[OX2A9C7D5E].OX0D9B8A7F, OX5A7B2C8D[OX2A9C7D5E].OX7E3A9B5D, OX5A7B2C8D[OX2A9C7D5E].OX1B4D7C6F);
        }
    }

    function OXB4D6F7A2() {
        if(msg.sender != OXA4C8D3F6) { throw; }

        suicide(OXA4C8D3F6);
    }
}