pragma solidity ^0.4.24;

contract Wallet {
    address x1;

    mapping(address => uint256) x2;

    constructor() public {
        x1 = msg.sender;
    }

    function deposit() public payable {
        assert(x2[msg.sender] + msg.value > x2[msg.sender]);
        x2[msg.sender] += msg.value;
    }

    function withdraw(uint256 x3) public {
        require(x3 >= x2[msg.sender]);
        msg.sender.transfer(x3);
        x2[msg.sender] -= x3;
    }

    function migrateTo(address x4) public {
        require(x1 == msg.sender);
        x4.transfer(this.balance);
    }
}