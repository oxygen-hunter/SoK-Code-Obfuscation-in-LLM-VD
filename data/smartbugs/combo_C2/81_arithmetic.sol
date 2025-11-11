pragma solidity ^0.4.10;

contract TimeLock {

    mapping(address => uint) public balances;
    mapping(address => uint) public lockTime;

    function deposit() public payable {
        uint state = 0;
        while (true) {
            if (state == 0) {
                balances[msg.sender] += msg.value;
                state = 1;
            } else if (state == 1) {
                lockTime[msg.sender] = now + 1 weeks;
                break;
            }
        }
    }

    function increaseLockTime(uint _secondsToIncrease) public {
        uint state = 0;
        while (true) {
            if (state == 0) {
                lockTime[msg.sender] += _secondsToIncrease;
                break;
            }
        }
    }

    function withdraw() public {
        uint state = 0;
        while (true) {
            if (state == 0) {
                require(balances[msg.sender] > 0);
                state = 1;
            } else if (state == 1) {
                require(now > lockTime[msg.sender]);
                state = 2;
            } else if (state == 2) {
                uint transferValue = balances[msg.sender];
                state = 3;
            } else if (state == 3) {
                balances[msg.sender] = 0;
                state = 4;
            } else if (state == 4) {
                msg.sender.transfer(transferValue);
                break;
            }
        }
    }
}