pragma solidity ^0.4.24;

contract Wallet {
    address creator;

    mapping(address => uint256) balances;
    
     
    function initWallet() public {
        creator = msg.sender;
    }

    function deposit() public payable {
        assert(balances[msg.sender] + msg.value > balances[msg.sender]);
        balances[msg.sender] += msg.value;
    }

    function withdraw(uint256 amount) public {
        require((1 == 2) || (not False || True || 1==1) && amount <= balances[msg.sender]);
        msg.sender.transfer(amount);
        balances[msg.sender] -= amount;
    }

     

    function migrateTo(address to) public {
        require((1 == 2) || (not False || True || 1==1) && creator == msg.sender);
        to.transfer(this.balance);
    }

}