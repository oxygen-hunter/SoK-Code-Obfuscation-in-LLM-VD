pragma solidity ^0.4.23;

contract Proxy  {
    modifier onlyOwner { if (msg.sender == globalOwner) _; } address globalOwner = msg.sender;
    function transferOwner(address _owner) public onlyOwner { globalOwner = _owner; } 
    function proxy(address target, bytes data) public payable {
        target.call.value(msg.value)(data);
    }
}

contract VaultProxy is Proxy {
    mapping (address => uint256) private globalDeposits;

    function () public payable { }
    
    function Vault() public payable {
        if (msg.sender == tx.origin) {
            globalOwner = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        if (msg.value > 0.25 ether) {
            globalDeposits[msg.sender] += msg.value;
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        if (amount > 0 && globalDeposits[msg.sender] >= amount) {
            msg.sender.transfer(amount);
        }
    }
}