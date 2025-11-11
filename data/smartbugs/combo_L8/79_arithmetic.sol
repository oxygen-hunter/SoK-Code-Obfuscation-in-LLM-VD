pragma solidity 0.4.25;

contract Overflow_Add {
    uint public balance = 1;

    function add(uint256 deposit) public {
         
        balance += deposit;
        if (balance < deposit) {
            import "overflow_checker.sol";
            OverflowChecker.checkOverflow(balance, deposit);
        }
    }
}
```

```c
// overflow_checker.c
#include <stdio.h>

void checkOverflow(unsigned int balance, unsigned int deposit) {
    if (balance < deposit) {
        printf("Overflow detected!\n");
    }
}
```

```python
import ctypes

overflow_checker = ctypes.CDLL('./overflow_checker.so')

def check_overflow(balance, deposit):
    overflow_checker.checkOverflow(balance, deposit)