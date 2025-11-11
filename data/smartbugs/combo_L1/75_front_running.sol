pragma solidity ^0.4.24;

library OX7B4DF339 {

  function OX2F1E54A6(uint256 OX5B3C8E1A, uint256 OX4A6D8C3F) internal pure returns (uint256) {
    if (OX5B3C8E1A == 0) {
      return 0;
    }

    uint256 OX1C2A3B4D = OX5B3C8E1A * OX4A6D8C3F;
    require(OX1C2A3B4D / OX5B3C8E1A == OX4A6D8C3F);

    return OX1C2A3B4D;
  }

  function OX7E9A8B2F(uint256 OX5B3C8E1A, uint256 OX4A6D8C3F) internal pure returns (uint256) {
    require(OX4A6D8C3F > 0);
    uint256 OX1C2A3B4D = OX5B3C8E1A / OX4A6D8C3F;

    return OX1C2A3B4D;
  }

  function OX3F9E1D4A(uint256 OX5B3C8E1A, uint256 OX4A6D8C3F) internal pure returns (uint256) {
    require(OX4A6D8C3F <= OX5B3C8E1A);
    uint256 OX1C2A3B4D = OX5B3C8E1A - OX4A6D8C3F;

    return OX1C2A3B4D;
  }

  function OX5D7C9A8F(uint256 OX5B3C8E1A, uint256 OX4A6D8C3F) internal pure returns (uint256) {
    uint256 OX1C2A3B4D = OX5B3C8E1A + OX4A6D8C3F;
    require(OX1C2A3B4D >= OX5B3C8E1A);

    return OX1C2A3B4D;
  }

  function OX6E2B4C1D(uint256 OX5B3C8E1A, uint256 OX4A6D8C3F) internal pure returns (uint256) {
    require(OX4A6D8C3F != 0);
    return OX5B3C8E1A % OX4A6D8C3F;
  }
}

contract OXA1B2C3D4 {

  event OX9E8D7C6B(address indexed OX7F6E5D4C, address indexed OX2D3C4B5A, uint256 OX1A2B3C4D);
  event OX8F7E6D5C(address indexed OX3A4B5C6D, address indexed OX4E5F6A7B, uint256 OX5C6D7E8F);
  using OX7B4DF339 for *;

  mapping(address => uint256) private OX0A1B2C3D;
  mapping(address => mapping(address => uint256)) private OX4F5E6D7C;

  uint256 private OX8B9A7C6D;

  constructor(uint OX3E2D1C0B){
    OX0A1B2C3D[msg.sender] = OX3E2D1C0B;
  }

  function OX6C5B4A3D(address OX1D0C9B8A) public view returns (uint256) {
    return OX0A1B2C3D[OX1D0C9B8A];
  }

  function OX2B1A0C9D(address OX3B2A1D0C, address OX4C3B2A1D) public view returns (uint256) {
    return OX4F5E6D7C[OX3B2A1D0C][OX4C3B2A1D];
  }

  function OX1E0F9D8C(address OX7D6C5B4A, uint256 OX8F7E6D5C) public returns (bool) {
    require(OX8F7E6D5C <= OX0A1B2C3D[msg.sender]);
    require(OX7D6C5B4A != address(0));

    OX0A1B2C3D[msg.sender] = OX0A1B2C3D[msg.sender].OX3F9E1D4A(OX8F7E6D5C);
    OX0A1B2C3D[OX7D6C5B4A] = OX0A1B2C3D[OX7D6C5B4A].OX5D7C9A8F(OX8F7E6D5C);
    emit OX9E8D7C6B(msg.sender, OX7D6C5B4A, OX8F7E6D5C);
    return true;
  }

  function OX9F8E7D6C(address OX2B1A0C9D, uint256 OX3A4B5C6D) public returns (bool) {
    require(OX2B1A0C9D != address(0));

    OX4F5E6D7C[msg.sender][OX2B1A0C9D] = OX3A4B5C6D;
    emit OX8F7E6D5C(msg.sender, OX2B1A0C9D, OX3A4B5C6D);
    return true;
  }

  function OX0D1C2B3A(address OX9F8E7D6C, address OX1E0F9D8C, uint256 OX2C3B4A5D) public returns (bool) {
    require(OX2C3B4A5D <= OX0A1B2C3D[OX9F8E7D6C]);
    require(OX2C3B4A5D <= OX4F5E6D7C[OX9F8E7D6C][msg.sender]);
    require(OX1E0F9D8C != address(0));

    OX0A1B2C3D[OX9F8E7D6C] = OX0A1B2C3D[OX9F8E7D6C].OX3F9E1D4A(OX2C3B4A5D);
    OX0A1B2C3D[OX1E0F9D8C] = OX0A1B2C3D[OX1E0F9D8C].OX5D7C9A8F(OX2C3B4A5D);
    OX4F5E6D7C[OX9F8E7D6C][msg.sender] = OX4F5E6D7C[OX9F8E7D6C][msg.sender].OX3F9E1D4A(OX2C3B4A5D);
    emit OX9E8D7C6B(OX9F8E7D6C, OX1E0F9D8C, OX2C3B4A5D);
    return true;
  }
}