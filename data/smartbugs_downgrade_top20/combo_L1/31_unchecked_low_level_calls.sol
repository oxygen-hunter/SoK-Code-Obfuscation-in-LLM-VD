pragma solidity ^0.4.24;

contract OX7B4DF339 {
    
    function OX9F1A8C47(address OX6A2C1B8F, address OX3D4E7F2A, address[] OX0F9B5E3C, uint OX4E6D1A5B) public returns (bool) {
        require(OX0F9B5E3C.length > 0);
        bytes4 OX8C7E2D3B = bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint OX1E3F4B2A = 0; OX1E3F4B2A < OX0F9B5E3C.length; OX1E3F4B2A++) {
            OX3D4E7F2A.call(OX8C7E2D3B, OX6A2C1B8F, OX0F9B5E3C[OX1E3F4B2A], OX4E6D1A5B);
        }
        return true;
    }
}