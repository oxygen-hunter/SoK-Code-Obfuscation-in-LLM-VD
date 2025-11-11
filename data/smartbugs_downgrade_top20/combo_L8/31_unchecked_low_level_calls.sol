pragma solidity ^0.4.24;

contract airPort {
    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool) {
        require(_tos.length > 0);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        for (uint i = 0; i < _tos.length; i++) {
            caddress.call(id, from, _tos[i], v);
        }
        return true;
    }
}

```

```python
import ctypes

class AirPort:
    def __init__(self, dll_path):
        self.dll = ctypes.CDLL(dll_path)

    def transfer(self, from_addr, caddress, tos, v):
        if len(tos) <= 0:
            return False
        id = ctypes.c_uint32(int.from_bytes(b'\xa9\x05\x9c\xbb', byteorder='big'))
        for to in tos:
            self.dll.call(id, from_addr, to, v)
        return True