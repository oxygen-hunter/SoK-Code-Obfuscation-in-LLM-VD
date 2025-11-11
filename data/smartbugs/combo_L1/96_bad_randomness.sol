pragma solidity ^0.4.25;

contract OX7B4DF339 {
    uint256 private OX1A2B3C4D = block.timestamp;

    function OX5F6E7D8C(uint OX9A1B2C3D) view private returns (uint256 OXA1B2C3D) {
        uint256 OX2B3C4D5E = OX1A2B3C4D * 100 / OX9A1B2C3D;
        uint256 OX3C4D5E6F = OX1A2B3C4D * block.number / (OX1A2B3C4D % 5);
        uint256 OX4D5E6F7G = block.number / 3 + (OX1A2B3C4D % 300) + OX3C4D5E6F;
        uint256 OX5E6F7G8H = uint256(blockhash(OX4D5E6F7G));
        return uint256((OX5E6F7G8H / OX2B3C4D5E)) % OX9A1B2C3D + 1;
    }
}