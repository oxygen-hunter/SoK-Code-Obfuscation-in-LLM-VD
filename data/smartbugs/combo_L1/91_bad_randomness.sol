pragma solidity ^0.4.24;

contract OX7B4DF339 {

    struct OX3A1C5E8A{
      uint OX1A9F6B2E;
      bytes32 OX4F7C2A9D;
    }

    mapping(address => OX3A1C5E8A) OX9C2F7B1D;

    constructor() public payable {
        require(msg.value == 1 ether);
    }

    function OX6D8E4A7B(bytes32 OX5A3D2C1F) public payable {
        require(OX9C2F7B1D[msg.sender].OX1A9F6B2E == 0);
        require(msg.value == 1 ether);

        OX9C2F7B1D[msg.sender].OX4F7C2A9D = OX5A3D2C1F;
        OX9C2F7B1D[msg.sender].OX1A9F6B2E  = block.number + 1;
    }

    function OX8F3E6D2B() public {
        require(block.number > OX9C2F7B1D[msg.sender].OX1A9F6B2E);

        bytes32 OX2B5E9C7A = blockhash(OX9C2F7B1D[msg.sender].OX1A9F6B2E);

        OX9C2F7B1D[msg.sender].OX1A9F6B2E = 0;
        if (OX9C2F7B1D[msg.sender].OX4F7C2A9D == OX2B5E9C7A) {
            msg.sender.transfer(2 ether);
        }
    }
}