pragma solidity ^0.4.24;

contract Wallet {
    address creator;

    mapping(address => uint256) balances;
    
     
    function initWallet() public {
        creator = msg.sender;
    }

    function deposit() public payable {
        uint256 currentBalance = balances[msg.sender];
        uint256 newValue = msg.value;
        assert(currentBalance + newValue > currentBalance);
        balances[msg.sender] = currentBalance + newValue;
    }

    function withdraw(uint256 amount) public {
        uint256 userBalance = balances[msg.sender];
        require(amount <= userBalance);
        msg.sender.transfer(amount);
        balances[msg.sender] = userBalance - amount;
    }

     
    function migrateTo(address to) public {
        address contractOwner = creator;
        require(contractOwner == msg.sender);
        address targetAddress = to;
        targetAddress.transfer(this.balance);
    }

}