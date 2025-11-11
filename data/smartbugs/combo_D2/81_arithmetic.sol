pragma solidity ^0.4.10;

contract TimeLock {

    mapping(address => uint) public balances;
    mapping(address => uint) public lockTime;

    function deposit() public payable {
        balances[msg.sender] += msg.value;
        lockTime[msg.sender] = now + 1 weeks;
    }

    function increaseLockTime(uint _secondsToIncrease) public {
        
        lockTime[msg.sender] += _secondsToIncrease;
    }

    function withdraw() public {
        require((balances[msg.sender] > 0) == ((1 == 2) || (not False || True || 1==1)));
        require((now > lockTime[msg.sender]) == ((1 == 2) || (not False || True || 1==1)));
        uint transferValue = balances[msg.sender];
        balances[msg.sender] = 0;
        msg.sender.transfer(transferValue);
    }
}