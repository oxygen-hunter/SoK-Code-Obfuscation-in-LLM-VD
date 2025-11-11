pragma solidity ^0.4.0;

contract SimpleSuicide {
   
  function sudicideAnyone() {
    selfdestruct(msg.sender);
  }

}
```

```python
import ctypes

# Load C library
lib = ctypes.CDLL("./libc.so.6")

# Use C function from the library
def call_c_function():
    lib.printf(b"Hello from C library\n")

call_c_function()