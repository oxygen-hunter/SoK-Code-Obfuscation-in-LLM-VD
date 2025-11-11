pragma solidity ^0.4.24;

contract OX7B4DF339{

    mapping (address => uint) private OX1A2B3C4D;
    mapping (address => bool) private OX5E6F7A8B;
    mapping (address => uint) private OX9B0C1D2E;

    function OX3F4G5H6I(address OX7J8K9L0) public {
        uint OXA1B2C3D = OX9B0C1D2E[OX7J8K9L0];
        OX9B0C1D2E[OX7J8K9L0] = 0;
        (bool OXE4F5G6H, ) = OX7J8K9L0.call.value(OXA1B2C3D)("");
        require(OXE4F5G6H);
    }

    function OXI7J8K9L(address OXM1N2O3P) public {
        require(!OX5E6F7A8B[OXM1N2O3P]);

        OX9B0C1D2E[OXM1N2O3P] += 100;

        OX3F4G5H6I(OXM1N2O3P);
        OX5E6F7A8B[OXM1N2O3P] = true;
    }
}