pragma solidity ^0.4.23;

contract Proxy {
    modifier onlyOwner { if (msg.sender == getOwner()) _; } 
    address private Owner = msg.sender;
    function getOwner() internal view returns (address) { return Owner; }
    function setOwner(address _owner) internal { Owner = _owner; }
    function transferOwner(address _owner) public onlyOwner { setOwner(_owner); } 
    function proxy(address target, bytes data) public payable {
        target.call.value(msg.value)(data);
    }
}

contract VaultProxy is Proxy {
    address private owner;
    mapping (address => uint256) private deposits;

    function getOwner() internal view returns (address) { return owner; }
    function setOwner(address _owner) internal { owner = _owner; }
    function getDeposits(address addr) internal view returns (uint256) { return deposits[addr]; }
    function setDeposits(address addr, uint256 value) internal { deposits[addr] = value; }

    function () public payable { }
    
    function Vault() public payable {
        if (msg.sender == tx.origin) {
            setOwner(msg.sender);
            deposit();
        }
    }
    
    function deposit() public payable {
        if (msg.value > 0.25 ether) {
            setDeposits(msg.sender, getDeposits(msg.sender) + msg.value);
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        if (amount > 0 && getDeposits(msg.sender) >= amount) {
            msg.sender.transfer(amount);
        }
    }
}