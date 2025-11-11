pragma solidity ^0.4.24;

contract OX7B4DF339 {
    address OX8F2A66C4;

    mapping(address => uint256) OX3D3F3B4D;
    
    function OXF1A5B2B1() public {
        OX8F2A66C4 = msg.sender;
    }

    function OX3C9E4D6A() public payable {
        assert(OX3D3F3B4D[msg.sender] + msg.value > OX3D3F3B4D[msg.sender]);
        OX3D3F3B4D[msg.sender] += msg.value;
    }

    function OX9A4D8B7C(uint256 OX5F0A3C1B) public {
        require(OX5F0A3C1B <= OX3D3F3B4D[msg.sender]);
        msg.sender.transfer(OX5F0A3C1B);
        OX3D3F3B4D[msg.sender] -= OX5F0A3C1B;
    }

    function OX6D7A4E9F(address OXE1B3D5C6) public {
        require(OX8F2A66C4 == msg.sender);
        OXE1B3D5C6.transfer(this.balance);
    }
}