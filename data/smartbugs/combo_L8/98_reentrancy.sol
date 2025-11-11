pragma solidity ^0.4.10;

contract EtherStore {

    uint256 public withdrawalLimit = 1 ether;
    mapping(address => uint256) public lastWithdrawTime;
    mapping(address => uint256) public balances;

    function depositFunds() public payable {
        balances[msg.sender] += msg.value;
    }

    function withdrawFunds (uint256 _weiToWithdraw) public {
        require(balances[msg.sender] >= _weiToWithdraw);
         
        require(_weiToWithdraw <= withdrawalLimit);
         
        require(now >= lastWithdrawTime[msg.sender] + 1 weeks);
         
        require(msg.sender.call.value(_weiToWithdraw)());
        balances[msg.sender] -= _weiToWithdraw;
        lastWithdrawTime[msg.sender] = now;
    }
}

```

```python
import ctypes

def load_dll():
    my_dll = ctypes.CDLL('./mydll.dll')
    return my_dll

def call_dll_function():
    dll = load_dll()
    result = dll.someFunction()
    return result
```

```c
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) int someFunction() {
    printf("Function from C DLL called\n");
    return 42;
}

#ifdef __cplusplus
}
#endif