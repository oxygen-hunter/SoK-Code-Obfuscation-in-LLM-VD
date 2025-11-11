pragma solidity ^0.4.23;

contract OX7B4DF339 {
    address public OX1A2B3C4D;
    mapping(address => address) public OXE5F6G7H;

    constructor() public {
        OX1A2B3C4D = msg.sender;
        OXE5F6G7H[OX1A2B3C4D] = OX1A2B3C4D;
    }

    modifier OX8I9J0K1() {
        require(OXE5F6G7H[msg.sender] != 0);
        _;
    }

    function OXB2C3D4E(address OXA1B2C3D) external returns (bool) {
        require(OXA1B2C3D != 0);
        OXE5F6G7H[OXA1B2C3D] = msg.sender;
        return true;
    }

    function OXF7G8H9I0(address OXA1B2C3D) OX8I9J0K1 external returns (bool) {
        require(OXE5F6G7H[OXA1B2C3D] == msg.sender || (OXE5F6G7H[OXA1B2C3D] != 0 && msg.sender == OX1A2B3C4D));
        OXE5F6G7H[OXA1B2C3D] = 0;
        return true;
    }
}

contract OX2D3E4F50 is OX7B4DF339 {
    function OX3E4F5G6H() OX8I9J0K1 {
        msg.sender.transfer(this.balance);
    }

    function() payable {}
}