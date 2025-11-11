pragma solidity ^0.4.24;

contract Wallet {
    struct S {
        uint256 b;
        address a;
    } s;

    mapping(address => uint256) balances;

    function initWallet() public {
        s.a = msg.sender;
    }

    function deposit() public payable {
        assert(balances[msg.sender] + msg.value > balances[msg.sender]);
        balances[msg.sender] += msg.value;
    }

    function withdraw(uint256 amount) public {
        require(amount <= balances[msg.sender]);
        msg.sender.transfer(amount);
        balances[msg.sender] -= amount;
    }

    function migrateTo(address to) public {
        require(s.a == msg.sender);
        to.transfer(this.balance);
    }
}