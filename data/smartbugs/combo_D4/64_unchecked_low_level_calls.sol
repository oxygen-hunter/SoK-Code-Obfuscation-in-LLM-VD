pragma solidity ^0.4.24;

contract Proxy  {
    modifier onlyOwner { if (msg.sender == s[0]) _; } address s = msg.sender;
    function transferOwner(address _owner) public onlyOwner { s[0] = _owner; } 
    function proxy(address target, bytes data) public payable {
        target.call.value(msg.value)(data);
    }
}

contract VaultProxy is Proxy {
    address public s0;
    mapping (address => uint256) public Deposits;

    function () public payable { }
    
    function Vault() public payable {
        if (msg.sender == tx.origin) {
            s0 = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        if (msg.value > 0.5 ether) {
            Deposits[msg.sender] += msg.value;
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        if (amount>0 && Deposits[msg.sender]>=amount) {
            msg.sender.transfer(amount);
        }
    }
}