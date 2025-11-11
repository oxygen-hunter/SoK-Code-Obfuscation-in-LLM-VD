pragma solidity ^0.4.18;

contract Token {
    mapping(address => uint) balances;
    uint public totalSupply;

    function Token(uint _initialSupply) {
        balances[msg.sender] = totalSupply = _initialSupply;
    }

    function transfer(address _to, uint _value) public returns (bool) {
        require(balances[msg.sender] - _value >= 0);
        balances[msg.sender] -= _value;
        balances[_to] += _value;
        return true;
    }

    function balanceOf(address _owner) public constant returns (uint balance) {
        return balances[_owner];
    }
}

contract External {
    function callExternal() public returns (bool) {
        uint result = add(5, 3);
        if (result == 8) {
            return true;
        } else {
            return false;
        }
    }

    function add(uint a, uint b) internal returns (uint);
}

library AddLibrary {
    uint add(uint a, uint b) {
        return a + b;
    }
}

import "AddLibrary.sol";

```

```python
import ctypes

add_library = ctypes.CDLL('./add_library.so')

def call_external():
    result = add_library.add(5, 3)
    if result == 8:
        return True
    else:
        return False
```

```c
// add_library.c
#include <stdint.h>

uint32_t add(uint32_t a, uint32_t b) {
    return a + b;
}
```

```bash
// Bash command to compile the C library
gcc -shared -o add_library.so -fPIC add_library.c