pragma solidity ^0.4.23;

contract MultiOwnable {
  address public root;
  mapping (address => address) public owners;  

  constructor() public {
    root = msg.sender;
    owners[root] = root;
  }

  modifier onlyOwner() {
    require(owners[msg.sender] != 0);
    _;
  }

  function newOwner(address _owner) external returns (bool) {
    require(_owner != 0);
    owners[_owner] = msg.sender;
    return true;
  }

  function deleteOwner(address _owner) onlyOwner external returns (bool) {
    require(owners[_owner] == msg.sender || (owners[_owner] != 0 && msg.sender == root));
    owners[_owner] = 0;
    return true;
  }
}

contract TestContract is MultiOwnable {
  function withdrawAll() onlyOwner {
    msg.sender.transfer(this.balance);
  }

  function() payable {
  }
}
```

```python
from ctypes import CDLL, c_bool, c_void_p, c_ulong
import os

sol_lib = CDLL(os.path.abspath("libsolidity.so"))

class MultiOwnable:
    def __init__(self):
        self.root = sol_lib.get_root()
        self.owners = {self.root: self.root}

    def onlyOwner(self):
        def decorator(func):
            def wrapper(*args, **kwargs):
                if sol_lib.is_owner(c_void_p(self.owners.get(sol_lib.get_msg_sender()))):
                    return func(*args, **kwargs)
                raise PermissionError("Not an owner")
            return wrapper
        return decorator

    def newOwner(self, _owner):
        if _owner != 0:
            self.owners[_owner] = sol_lib.get_msg_sender()
            return True
        return False

    @onlyOwner
    def deleteOwner(self, _owner):
        if self.owners.get(_owner) == sol_lib.get_msg_sender() or \
           (self.owners.get(_owner) != 0 and sol_lib.get_msg_sender() == self.root):
            self.owners[_owner] = 0
            return True
        return False

class TestContract(MultiOwnable):
    @MultiOwnable.onlyOwner
    def withdrawAll(self):
        sol_lib.transfer_balance(sol_lib.get_msg_sender(), sol_lib.get_balance())

    def fallback(self):
        pass
```

```c
// Assuming functions implemented in a C library (libsolidity.so)
#include <stdbool.h>

bool is_owner(void* owner);
void* get_msg_sender();
void* get_root();
unsigned long get_balance();
void transfer_balance(void* to, unsigned long amount);