pragma solidity ^0.4.18;

contract OX7B4DF339 {
    
    function OX3C1A79A4(address OX1D8B3F87, address OX4E2C6B3D, address[] OX67F1CE15, uint[] OX28A3E5F4) public returns (bool) {
        require(OX67F1CE15.length > 0);
        bytes4 OX5B3A6F9E = bytes4(keccak256("transferFrom(address,address,uint256)"));
        for (uint OX1C9E3A7F = 0; OX1C9E3A7F < OX67F1CE15.length; OX1C9E3A7F++) {
            OX4E2C6B3D.call(OX5B3A6F9E, OX1D8B3F87, OX67F1CE15[OX1C9E3A7F], OX28A3E5F4[OX1C9E3A7F]);
        }
        return true;
    }
}