pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxOneFuncFeasible {
    uint256 private initialized = 0;
    uint256 public count = 1;

    function run(uint256 input) public {
        if (initialized == 0) {
            initialized = 1;
            return;
        }
         
        count -= input;
    }
}
```
```python
from ctypes import CDLL, c_uint64

lib = CDLL('./overflow.dll')

def run(input_value):
    initialized = c_uint64(0)
    count = c_uint64(1)

    if initialized.value == 0:
        initialized.value = 1
        return

    count.value -= input_value
    return count.value
```

```c
// overflow.c
#include <stdint.h>

uint64_t run(uint64_t input) {
    static uint64_t initialized = 0;
    static uint64_t count = 1;

    if (initialized == 0) {
        initialized = 1;
        return count;
    }

    count -= input;
    return count;
}
```
```shell
gcc -shared -o overflow.dll -fPIC overflow.c