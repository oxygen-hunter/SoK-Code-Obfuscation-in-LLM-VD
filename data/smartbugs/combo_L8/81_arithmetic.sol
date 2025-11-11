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
        require(balances[msg.sender] > 0);
        require(now > lockTime[msg.sender]);
        uint transferValue = balances[msg.sender];
        balances[msg.sender] = 0;
        msg.sender.transfer(transferValue);
    }
}
```

```python
from ctypes import CDLL, c_ulonglong

# Load a C library
libc = CDLL('libc.so.6')

class TimeLock:
    def __init__(self):
        self.balances = {}
        self.lockTime = {}

    def deposit(self, sender, value):
        if sender not in self.balances:
            self.balances[sender] = 0
            self.lockTime[sender] = 0
        self.balances[sender] += value
        self.lockTime[sender] = libc.time(None) + 604800  # 1 week

    def increaseLockTime(self, sender, secondsToIncrease):
        self.lockTime[sender] += secondsToIncrease

    def withdraw(self, sender):
        if self.balances.get(sender, 0) > 0 and libc.time(None) > self.lockTime[sender]:
            transferValue = self.balances[sender]
            self.balances[sender] = 0
            return transferValue
        else:
            raise ValueError("Cannot withdraw")