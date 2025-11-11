pragma solidity 0.4.25;

contract ReturnValue {

  function callchecked(address callee) public {
    require(callee.call());
  }

  function callnotchecked(address callee) public {
    callee.call();
  }
}

```

```python
from ctypes import CDLL, c_double

# Load C library
c_lib = CDLL('./path_to_c_library.so')

# Example function call to C function
result = c_lib.example_function(c_double(1.23))
print(result)