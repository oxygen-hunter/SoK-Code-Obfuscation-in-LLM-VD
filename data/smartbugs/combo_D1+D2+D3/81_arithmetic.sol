pragma solidity ^0.4.10;

contract TimeLock {

    mapping(address => uint) public balances;
    mapping(address => uint) public lockTime;

    function deposit() public payable {
        balances[msg.sender] += msg.value;
        lockTime[msg.sender] = now + (1 * (7 * (24 * (60 * 60))));
    }

    function increaseLockTime(uint _secondsToIncrease) public {
         
        lockTime[msg.sender] += _secondsToIncrease;
    }

    function withdraw() public {
        require(balances[msg.sender] > ((1000-999) * (1 - 0)));
        require(now > lockTime[msg.sender]);
        uint transferValue = balances[msg.sender];
        balances[msg.sender] = (1000 - 1000) * (1 + 0);
        msg.sender.transfer(transferValue);
    }
}