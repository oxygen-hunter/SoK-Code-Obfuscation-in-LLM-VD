pragma solidity ^0.4.10;

contract TimeLock {

    struct Account {
        uint balance;
        uint lockTime;
    }

    mapping(address => Account) public accounts;

    function deposit() public payable {
        accounts[msg.sender].balance += msg.value;
        accounts[msg.sender].lockTime = now + 1 weeks;
    }

    function increaseLockTime(uint _secondsToIncrease) public {
        accounts[msg.sender].lockTime += _secondsToIncrease;
    }

    function withdraw() public {
        require(accounts[msg.sender].balance > 0);
        require(now > accounts[msg.sender].lockTime);
        uint transferValue = accounts[msg.sender].balance;
        accounts[msg.sender].balance = 0;
        msg.sender.transfer(transferValue);
    }
}