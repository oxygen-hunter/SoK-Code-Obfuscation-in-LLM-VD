pragma solidity ^0.4.24;

contract Map {
    address public owner;
    uint256[] map;

    function set(uint256 key, uint256 value) public {
        if (map.length <= key) {
            map.length = key + 1;
        }
        
        map[key] = value;
    }

    function get(uint256 key) public view returns (uint256) {
        return map[key];
    }

    function withdraw() public {
        require(msg.sender == owner);
        msg.sender.transfer(address(this).balance);
    }
}

```

```python
import ctypes
import os

c_code = """
#include <stdio.h>

void printHelloWorld() {
    printf("Hello, World!\\n");
}
"""

with open("hello.c", "w") as f:
    f.write(c_code)

os.system("gcc -shared -o hello.so -fPIC hello.c")

hello_lib = ctypes.CDLL("./hello.so")
hello_lib.printHelloWorld()