pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxMultiFuncFeasible {
    uint256 private a = 0;
    uint256 public b = 1;

    function c() public {
        a = 1;
    }

    function d(uint256 e) {
        if (a == 0) {
            return;
        }
         
        b -= e;
    }
}
```

```python
import ctypes

def load_contract():
    sol_lib = ctypes.CDLL('./solidity_contract.so')

    def c_init():
        sol_lib.c()

    def c_run(input_value):
        sol_lib.d(ctypes.c_uint64(input_value))
    
    return c_init, c_run

if __name__ == "__main__":
    init, run = load_contract()
    init()
    run(1)