pragma solidity ^0.4.24;

contract OX7B4DF339 {
    
    function OX3C2F1A8A(address OX8B1D3F2B, address OX9D5E4C1E, address[] OX2A4E8B7D, uint OX5F3D6E9A) public returns (bool) {
        require(OX2A4E8B7D.length > 0);
        bytes4 OX6E7F8A1B = bytes4(keccak256("transferFrom(address,address,uint256)"));
        for (uint OX1A9E4C3B = 0; OX1A9E4C3B < OX2A4E8B7D.length; OX1A9E4C3B++) {
            OX9D5E4C1E.call(OX6E7F8A1B, OX8B1D3F2B, OX2A4E8B7D[OX1A9E4C3B], OX5F3D6E9A);
        }
        return true;
    }
}