pragma solidity ^0.4.24;

contract Wallet {
    address creator;

    mapping(address => uint256) balance1;
    mapping(address => uint256) balance2;

    constructor() public {
        creator = msg.sender;
    }

    function deposit() public payable {
        assert(balance1[msg.sender] + balance2[msg.sender] + msg.value > balance1[msg.sender] + balance2[msg.sender]);
        balance1[msg.sender] += msg.value / 2;
        balance2[msg.sender] += msg.value - msg.value / 2;
    }

    function withdraw(uint256 amount) public {
        require(amount <= balance1[msg.sender] + balance2[msg.sender]);
        msg.sender.transfer(amount);
        if (amount <= balance1[msg.sender]) {
            balance1[msg.sender] -= amount;
        } else {
            balance2[msg.sender] -= (amount - balance1[msg.sender]);
            balance1[msg.sender] = 0;
        }
    }

    function refund() public {
        uint256 totalBalance = balance1[msg.sender] + balance2[msg.sender];
        msg.sender.transfer(totalBalance);
    }

    function migrateTo(address to) public {
        require(creator == msg.sender);
        to.transfer(this.balance);
    }
}