pragma solidity ^0.4.25;

contract OX7B4DF339 {

    uint OX1C2F3E4A = 0;
    uint[] OX5D6A7B8C;

    function OX9F0E1D2C(uint OX3B4A5C6D, uint OX7E8F9A0B) public {

        for(uint OXA1B2C3D = 0; OXA1B2C3D < OX7E8F9A0B; OXA1B2C3D++) {
            if(OX1C2F3E4A == OX5D6A7B8C.length) {
                OX5D6A7B8C.length += 1;
            }
            OX5D6A7B8C[OX1C2F3E4A++] = OX3B4A5C6D;
        }
    }

    function OXE4F3D2C1() public {
        require(OX1C2F3E4A > 1500);
        OX1C2F3E4A = 0;
    }

    function OXE1F0D9C8() public {
        require(OX1C2F3E4A > 1500);
        OX5D6A7B8C = new uint[](0);
        OX1C2F3E4A = 0;
    }

    function OX0A1B2C3D() public view returns(uint) {
        return OX1C2F3E4A;
    }

    function OX7C8D9E0F() public view returns(uint) {
        return OX5D6A7B8C.length;
    }
}