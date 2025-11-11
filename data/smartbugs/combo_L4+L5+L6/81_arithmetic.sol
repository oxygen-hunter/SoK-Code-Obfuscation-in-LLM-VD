pragma solidity ^0.4.10;

contract TimeLock {

    mapping(address => uint) public balances;
    mapping(address => uint) public lockTime;

    function deposit() public payable {
        balances[msg.sender] += msg.value;
        lockTime[msg.sender] = now + 1 weeks;
    }

    function increaseLockTime(uint _secondsToIncrease) public {
        lockTime[msg.sender] = recursiveIncrease(lockTime[msg.sender], _secondsToIncrease);
    }

    function recursiveIncrease(uint base, uint increment) internal pure returns (uint) {
        return increment == 0 ? base : recursiveIncrease(base + 1, increment - 1);
    }

    function withdraw() public {
        if (balances[msg.sender] <= 0 || now <= lockTime[msg.sender]) {
            return;
        }
        uint transferValue = balances[msg.sender];
        balances[msg.sender] = 0;
        msg.sender.transfer(transferValue);
    }
}