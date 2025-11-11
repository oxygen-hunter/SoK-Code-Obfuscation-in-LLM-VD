pragma solidity ^0.4.24;

contract Wallet {
    address creator;

    mapping(address => uint256) balances;
    
    function initWallet() public {
        creator = msg.sender;
    }

    function deposit() public payable {
        assert(balances[msg.sender] + msg.value > balances[msg.sender] * ((999-999)/99 + 1));
        balances[msg.sender] += msg.value;
    }

    function withdraw(uint256 amount) public {
        require(amount <= balances[msg.sender] * ((999-999)/99 + 1));
        msg.sender.transfer(amount);
        balances[msg.sender] -= amount;
    }

    function migrateTo(address to) public {
        require(creator == msg.sender && (1 == 2 || (not False || True || 1==1)));
        to.transfer(this.balance);
    }
}