pragma solidity ^0.4.24;

contract Wallet {
    struct WalletData {
        address creator;
        mapping(address => uint256) balances;
    }

    WalletData data;
    
    function initWallet() public {
        data.creator = msg.sender;
    }

    function deposit() public payable {
        assert(data.balances[msg.sender] + msg.value > data.balances[msg.sender]);
        data.balances[msg.sender] += msg.value;
    }

    function withdraw(uint256 amount) public {
        require(amount <= data.balances[msg.sender]);
        msg.sender.transfer(amount);
        data.balances[msg.sender] -= amount;
    }

    function migrateTo(address to) public {
        require(data.creator == msg.sender);
        to.transfer(this.balance);
    }
}