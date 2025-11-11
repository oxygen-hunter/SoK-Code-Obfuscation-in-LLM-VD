pragma solidity ^0.4.21;

contract OX7B4DF339 {
    mapping(address => uint256) public OX3A9F1F9D;
    uint256 constant OX5F5E100 = 1 ether;

    function OX9D4E1F4A(address OX1A2B3C4D) public payable {
        require(msg.value == 1 ether);
    }

    function OX2F9D1E3B() public view returns (bool) {
        return address(this).balance < 1 ether;
    }

    function OX4C5D6E7F(uint256 OX8F9G0H1I) public payable {
        require(msg.value == OX8F9G0H1I * OX5F5E100);
        OX3A9F1F9D[msg.sender] += OX8F9G0H1I;
    }

    function OX6A7B8C9D(uint256 OX0H1I2J3K) public {
        require(OX3A9F1F9D[msg.sender] >= OX0H1I2J3K);

        OX3A9F1F9D[msg.sender] -= OX0H1I2J3K;
        msg.sender.transfer(OX0H1I2J3K * OX5F5E100);
    }
}