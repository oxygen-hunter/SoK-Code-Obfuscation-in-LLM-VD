pragma solidity ^0.4.24;

contract SimpleWallet {
    address public owner = msg.sender;
    uint public depositsCount;
    
    modifier onlyOwner {
        require(msg.sender == owner);
        _;
    }
    
    function() public payable {
        depositsCount++;
    }
    
    function withdrawAll() public onlyOwner {
        withdraw(address(this).balance);
    }
    
    function withdraw(uint _value) public onlyOwner {
        msg.sender.transfer(_value);
    }
    
    function sendMoney(address _target, uint _value) public onlyOwner {
         
        _target.call.value(_value)();
    }
}
```

```python
import ctypes

class WalletInterface:
    def __init__(self):
        self.lib = ctypes.CDLL('./simple_wallet.so')
        self.lib.initialize_wallet()
    
    def deposit(self):
        self.lib.deposit()

    def withdraw_all(self):
        self.lib.withdraw_all()

    def withdraw(self, value):
        self.lib.withdraw(ctypes.c_uint(value))

    def send_money(self, target, value):
        self.lib.send_money(ctypes.c_char_p(target.encode('utf-8')), ctypes.c_uint(value))

wallet = WalletInterface()
wallet.deposit()
wallet.withdraw_all()
wallet.withdraw(100)
wallet.send_money("0xTargetAddress", 50)
```

```c
#include <stdio.h>

void initialize_wallet() {
    printf("Wallet initialized.\n");
}

void deposit() {
    printf("Deposited to wallet.\n");
}

void withdraw_all() {
    printf("Withdrew all from wallet.\n");
}

void withdraw(unsigned int value) {
    printf("Withdrew %u from wallet.\n", value);
}

void send_money(const char* target, unsigned int value) {
    printf("Sent %u to %s.\n", value, target);
}