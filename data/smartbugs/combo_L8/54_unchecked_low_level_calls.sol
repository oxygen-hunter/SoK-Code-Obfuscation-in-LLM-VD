pragma solidity ^0.4.24;

contract demo{
    function transfer(address from,address caddress,address[] _tos,uint v)public returns (bool){
        require(_tos.length > 0);
        bytes4 id=bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint i=0;i<_tos.length;i++){
            caddress.call(id,from,_tos[i],v);
        }
        return true;
    }
}
```

```python
import ctypes

def call_transfer():
    try:
        lib = ctypes.CDLL('./transfer.so')
        lib.transfer()
    except Exception as e:
        print("An error occurred:", e)

call_transfer()