pragma solidity ^0.4.21;

contract OX7B4DF339 {
    uint8 OX1A2E3F57;

    function OX8C9D0EFA() public payable {
        require(msg.value == 1 ether);
         
        OX1A2E3F57 = uint8(keccak256(block.blockhash(block.number - 1), now));
    }

    function OX5F6G7H8I() public view returns (bool) {
        return address(this).balance == 0;
    }

    function OX2J3K4L5M(uint8 OX6N7O8P9Q) public payable {
        require(msg.value == 1 ether);

        if (OX6N7O8P9Q == OX1A2E3F57) {
            msg.sender.transfer(2 ether);
        }
    }
}