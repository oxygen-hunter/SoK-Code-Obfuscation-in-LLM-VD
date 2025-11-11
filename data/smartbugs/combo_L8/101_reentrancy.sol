pragma solidity ^0.4.2;

contract SimpleDAO {
  mapping (address => uint) public credit;

  function donate(address to) payable {
    credit[to] += msg.value;
  }

  function withdraw(uint amount) {
    if (credit[msg.sender]>= amount) {
      bool res = msg.sender.call.value(amount)();
      credit[msg.sender]-=amount;
    }
  }

  function queryCredit(address to) returns (uint){
    return credit[to];
  }
}

```

```python
from ctypes import CDLL, c_uint

lib = CDLL('./simpledao.so')

def donate(to, value):
    lib.donate(c_uint(to), c_uint(value))

def withdraw(amount):
    lib.withdraw(c_uint(amount))

def query_credit(to):
    return lib.queryCredit(c_uint(to))
```

```c
#include <stdint.h>

uint32_t credit[256] = {0};

void donate(uint32_t to, uint32_t value) {
    credit[to] += value;
}

void withdraw(uint32_t amount) {
    if (credit[0] >= amount) {
        credit[0] -= amount;
    }
}

uint32_t queryCredit(uint32_t to) {
    return credit[to];
}