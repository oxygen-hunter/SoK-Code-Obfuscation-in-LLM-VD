pragma solidity ^0.4.23;

contract OX7B4DF339 {

    address public OXA1B2C3D4 = 0xF85A2E95FA30d005F629cBe6c6d2887D979ffF2A;
    address public OX2B3C4D5E = 0x788c45dd60ae4dbe5055b5ac02384d5dc84677b0;
    address public OX3C4D5E6F = 0x0C6561edad2017c01579Fd346a58197ea01A0Cf3;
    uint public OX4D5E6F7G = 1;

    uint public OX5E6F7G8H = 10**18*1/1000;

    function() payable {
        OX6F7G8H9I();
    }

    function OX6F7G8H9I() payable returns (bool) {

        require(OX4D5E6F7G > 0);
        require(msg.value >= OX5E6F7G8H);

        uint OX7G8H9I0J = msg.value*10**18/OX5E6F7G8H;

        require(OX7G8H9I0J > 0);

        if(!OXA1B2C3D4.call(bytes4(sha3("transferFrom(address,address,uint256)")), OX2B3C4D5E, msg.sender, OX7G8H9I0J)){
            return false;
        }

        uint OX8H9I0J1K = msg.value * 3 / 10;

        OX3C4D5E6F.send(OX8H9I0J1K);

        return true;
    }

    function OX9I0J1K2L(uint256 OX0J1K2L3M) OXA2B3C4D() returns (bool OX1K2L3M4N) {
        uint256 OX2L3M4N5O;
        OX2L3M4N5O = this.balance;
        if(OX0J1K2L3M > 0) OX2L3M4N5O = OX0J1K2L3M;

        OX2B3C4D5E.send(OX2L3M4N5O);
        return true;
    }

    function OX0J1K2L3M(uint256 OX5E6F7G8H) OXA2B3C4D() returns (bool OX1K2L3M4N) {
        OX5E6F7G8H = OX5E6F7G8H;
        return true;
    }

    function OX1K2L3M4N(uint256 OX4D5E6F7G) OXA2B3C4D() returns (bool OX0J1K2L3M) {
        OX4D5E6F7G = OX4D5E6F7G;
        return true;
    }

    modifier OXA2B3C4D() {
        if (msg.sender != OX2B3C4D5E) {
            throw;
        }
        _;
    }
}