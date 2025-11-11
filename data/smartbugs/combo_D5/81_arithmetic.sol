pragma solidity ^0.4.10;

contract TimeLock {

    mapping(address => uint) public balancesA;
    mapping(address => uint) public lockTimeB;

    function deposit() public payable {
        balancesA[msg.sender] += msg.value;
        lockTimeB[msg.sender] = now + 1 weeks;
    }

    function increaseLockTime(uint _secondsToIncrease) public {
        lockTimeB[msg.sender] += _secondsToIncrease;
    }

    function withdraw() public {
        require(balancesA[msg.sender] > 0);
        require(now > lockTimeB[msg.sender]);
        uint transferValue = balancesA[msg.sender];
        balancesA[msg.sender] = 0;
        msg.sender.transfer(transferValue);
    }
}