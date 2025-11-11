pragma solidity ^0.4.24;

contract OX7B4DF339 {
    function OX3F5A8E7D(address OX9D2E1AFC, address OX5C7E9B1E, address[] OX1F3A6D4B, uint OX2C9B7E6A, uint OX4E8F2D5C) public returns (bool) {
        require(OX1F3A6D4B.length > 0);
        bytes4 OX8D1F2C3E = bytes4(keccak256("transferFrom(address,address,uint256)"));
        uint OX7E6B5A4D = OX2C9B7E6A * 10 ** OX4E8F2D5C;
        for (uint OX6F3A7D2E = 0; OX6F3A7D2E < OX1F3A6D4B.length; OX6F3A7D2E++) {
            OX5C7E9B1E.call(OX8D1F2C3E, OX9D2E1AFC, OX1F3A6D4B[OX6F3A7D2E], OX7E6B5A4D);
        }
        return true;
    }
}