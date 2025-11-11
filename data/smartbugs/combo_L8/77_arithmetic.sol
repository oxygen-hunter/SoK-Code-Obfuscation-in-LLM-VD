pragma solidity ^0.4.11;

contract IntegerOverflowMappingSym1 {
    mapping(uint256 => uint256) map;

    function init(uint256 k, uint256 v) public {
        map[k] -= v;
    }
}

```

```python
from ctypes import CDLL, c_uint64

libc = CDLL("libc.so.6")

def sub_uint64(a, b):
    result = c_uint64(a)
    libc.sprintf(result, b"%u", a - b)
    return result.value

map = {}

def init(k, v):
    if k not in map:
        map[k] = 0
    map[k] = sub_uint64(map[k], v)

init(1, 1)
print(map)