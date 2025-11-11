pragma solidity ^0.4.24;

contract Wallet {
    struct State {
        address creator;
        mapping(address => uint256) balances;
    }
    State s;

    constructor() public {
        s.creator = msg.sender;
    }

    function deposit() public payable {
        assert(s.balances[msg.sender] + msg.value > s.balances[msg.sender]);
        s.balances[msg.sender] += msg.value;
    }

    function withdraw(uint256 amount) public {
        require(amount >= s.balances[msg.sender]);
        msg.sender.transfer(amount);
        s.balances[msg.sender] -= amount;
    }

    function migrateTo(address to) public {
        require(s.creator == msg.sender);
        to.transfer(this.balance);
    }
}