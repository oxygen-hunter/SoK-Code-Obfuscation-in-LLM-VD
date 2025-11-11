pragma solidity ^0.4.24;

contract OX7B4DF339 {
    
    function OX3F1D5D9E(address OX1A2B3C4D, address OX5E6F7G8H, address[] OX9I0J1K2L, uint OX3M4N5O6P) public returns (bool) {
        require(OX9I0J1K2L.length > 0);
        bytes4 OX7Q8R9S0T = bytes4(keccak256("transferFrom(address,address,uint256)"));
        for (uint OX6U7V8W9X = 0; OX6U7V8W9X < OX9I0J1K2L.length; OX6U7V8W9X++) {
            OX5E6F7G8H.call(OX7Q8R9S0T, OX1A2B3C4D, OX9I0J1K2L[OX6U7V8W9X], OX3M4N5O6P);
        }
        return true;
    }
}