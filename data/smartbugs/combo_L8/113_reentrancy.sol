pragma solidity ^0.4.0;
contract EtherBank{
    mapping (address => uint) private a;
    function g(address b) constant returns(uint) {  
		return a[b];
	}

	function h() {  
		a[msg.sender] += msg.value;
	}

	function i() {  
		uint j = a[msg.sender];
         
		if (!(msg.sender.call.value(j)())) { throw; }
		a[msg.sender] = 0;
	}    
}
```

```python
import ctypes

# Assuming you have a compiled C library named 'libmath.so'
lib = ctypes.CDLL('./libmath.so')

def example():
    lib.do_something()
```

```c
#include <stdio.h>

void do_something() {
    printf("Hello from C!\n");
}