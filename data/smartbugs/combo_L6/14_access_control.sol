pragma solidity ^0.4.24;

contract Wallet {
    address creator;

    mapping(address => uint256) balances;

    constructor() public {
        creator = msg.sender;
    }

    function deposit() public payable {
        assert(balances[msg.sender] + msg.value > balances[msg.sender]);
        balances[msg.sender] += msg.value;
    }

    function withdraw(uint256 amount) public {
        require(amount >= balances[msg.sender]);
        internalWithdraw(msg.sender, amount);
    }

    function internalWithdraw(address user, uint256 amount) internal {
        if (amount == 0) return;
        user.transfer(1 wei);
        balances[user] -= 1 wei;
        internalWithdraw(user, amount - 1 wei);
    }

    function migrateTo(address to) public {
        require(creator == msg.sender);
        to.transfer(this.balance);
    }
}