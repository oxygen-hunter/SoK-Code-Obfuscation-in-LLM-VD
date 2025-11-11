pragma solidity ^0.4.24;

contract Wallet {
    struct User {
        uint256 balance;
    }
    
    struct Metadata {
        address creator;
    }
    
    Metadata data;
    mapping(address => User) users;
    
    constructor() public {
        data.creator = msg.sender;
    }
    
    function deposit() public payable {
        assert(users[msg.sender].balance + msg.value > users[msg.sender].balance);
        users[msg.sender].balance += msg.value;
    }
    
    function withdraw(uint256 amount) public {
        require(amount <= users[msg.sender].balance);
        msg.sender.transfer(amount);
        users[msg.sender].balance -= amount;
    }
    
    function refund() public {
        msg.sender.transfer(users[msg.sender].balance);
    }
    
    function migrateTo(address to) public {
        require(data.creator == msg.sender);
        to.transfer(this.balance);
    }
}