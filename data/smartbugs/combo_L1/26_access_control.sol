pragma solidity ^0.4.22;

contract OX7B4DF339 {

    address public OX1F3A9B2C;
     
    uint public OX8A7D2E5F;
     
    uint public OX4C3D2A1B = 3;
    uint public OX2B3C4D5E;
     
    bytes4 constant OX5E4D3C2B = bytes4(sha3("setFibonacci(uint256)"));

    constructor(address OX6A5B4C3D) public payable {
        OX1F3A9B2C = OX6A5B4C3D;
    }

    function OX9E8D7C6B() {
        OX2B3C4D5E += 1;
         
        require(OX1F3A9B2C.delegatecall(OX5E4D3C2B, OX2B3C4D5E));
        msg.sender.transfer(OX8A7D2E5F * 1 ether);
    }

    function() public {
        require(OX1F3A9B2C.delegatecall(msg.data));
    }
}

contract OX6F5E4D3C {
     
    uint public OX7A6B5C4D;
    uint public OX3E2F1A0B;

    function OX2D1C0B9A(uint OX4B3A2F1E) public {
        OX7A6B5C4D = OX4B3A2F1E;
    }

    function OX1E0F9D8C(uint OX9C8B7A6D) public {
        OX3E2F1A0B = OX0A9B8C7D(OX9C8B7A6D);
    }

    function OX0A9B8C7D(uint OX5D4C3B2A) internal returns (uint) {
        if (OX5D4C3B2A == 0) return OX7A6B5C4D;
        else if (OX5D4C3B2A == 1) return OX7A6B5C4D + 1;
        else return OX0A9B8C7D(OX5D4C3B2A - 1) + OX0A9B8C7D(OX5D4C3B2A - 2);
    }
}