pragma solidity ^0.4.24;

contract Wallet {
    mapping(address => uint256) balances;
    
    function initWallet() public {
        address creator = msg.sender;
        migrateTo(creator);
    }

    function deposit() public payable {
        uint256 newBalance = balances[msg.sender] + msg.value;
        assert(newBalance > balances[msg.sender]);
        balances[msg.sender] = newBalance;
    }

    function withdraw(uint256 amount) public {
        require(amount <= balances[msg.sender]);
        msg.sender.transfer(amount);
        balances[msg.sender] -= amount;
    }

    function migrateTo(address to) internal {
        require(to == msg.sender);
        to.transfer(this.balance);
    }
}