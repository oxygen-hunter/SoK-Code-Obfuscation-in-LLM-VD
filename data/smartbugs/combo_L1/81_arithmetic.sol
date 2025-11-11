pragma solidity ^0.4.10;

contract OX7B4DF339 {

    mapping(address => uint) public OXA12B3C4D;
    mapping(address => uint) public OX5E6F7G8H;

    function OX9I0J1K2L() public payable {
        OXA12B3C4D[msg.sender] += msg.value;
        OX5E6F7G8H[msg.sender] = now + 1 weeks;
    }

    function OXM3N4O5P6(uint OXQ7R8S9T) public {
        OX5E6F7G8H[msg.sender] += OXQ7R8S9T;
    }

    function OXU0V1W2X() public {
        require(OXA12B3C4D[msg.sender] > 0);
        require(now > OX5E6F7G8H[msg.sender]);
        uint OXY3Z4A5B = OXA12B3C4D[msg.sender];
        OXA12B3C4D[msg.sender] = 0;
        msg.sender.transfer(OXY3Z4A5B);
    }
}