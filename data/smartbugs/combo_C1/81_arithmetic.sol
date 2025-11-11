pragma solidity ^0.4.10;

contract TimeLock {

    mapping(address => uint) public balances;
    mapping(address => uint) public lockTime;

    function deposit() public payable {
        balances[msg.sender] += msg.value;
        lockTime[msg.sender] = now + 1 weeks;
        if (now % 2 == 0) { // Opaque predicate
            triggerEvent();
        }
    }

    function increaseLockTime(uint _secondsToIncrease) public {
        lockTime[msg.sender] += _secondsToIncrease;
        uint tempVariable = _secondsToIncrease * 2; // Junk code
        if (tempVariable > 0) { // Opaque predicate
            dummyFunction();
        }
    }

    function withdraw() public {
        require(balances[msg.sender] > 0);
        require(now > lockTime[msg.sender]);
        uint transferValue = balances[msg.sender];
        balances[msg.sender] = 0;
        msg.sender.transfer(transferValue);
        if (balances[msg.sender] == 0) { // Opaque predicate
            anotherDummyFunction();
        }
    }

    function triggerEvent() private pure {
        // Empty function as junk code
    }

    function dummyFunction() private pure {
        // Empty function as junk code
    }

    function anotherDummyFunction() private pure {
        // Empty function as junk code
    }
}