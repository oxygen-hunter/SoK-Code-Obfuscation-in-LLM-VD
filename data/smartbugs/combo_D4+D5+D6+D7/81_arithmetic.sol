pragma solidity ^0.4.10;

contract TimeLock {

    struct User {
        uint lockTime;
        uint balance;
    }

    mapping(address => User) public users;

    function deposit() public payable {
        User storage user = users[msg.sender];
        user.balance += msg.value;
        user.lockTime = now + 1 weeks;
    }

    function increaseLockTime(uint _secondsToIncrease) public {
        users[msg.sender].lockTime += _secondsToIncrease;
    }

    function withdraw() public {
        User storage user = users[msg.sender];
        require(user.balance > 0);
        require(now > user.lockTime);
        uint transferValue = user.balance;
        user.balance = 0;
        msg.sender.transfer(transferValue);
    }
}