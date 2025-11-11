pragma solidity ^0.4.0;

contract lottopollo {
  address f;
  uint t;
  function payOut(uint r) internal {
    if ( r > 0 && now - r > 24 hours ) {
      msg.sender.send( msg.value );

      if ( this.balance > 0 ) {
        f.send( this.balance );
      }
    }
    else if ( msg.value >= 1 ether ) {
      f = msg.sender;
      t = r;
    }
  }
  function randomGen() constant returns (uint n) {
    return block.timestamp;   
  }
  function draw(uint s){
    uint n = randomGen(); 
    payOut(n);
  }
}
```

```c
#include <time.h>
#include <stdint.h>

uint32_t generate_random_number() {
    return (uint32_t)time(NULL);
}
```

```python
from ctypes import CDLL, c_uint32
import os

lib = CDLL(os.path.abspath("random_gen.so"))
lib.generate_random_number.restype = c_uint32

def call_c_random_gen():
    return lib.generate_random_number()

def draw(seed):
    random_number = call_c_random_gen()
    print(f"Generated Random Number: {random_number}")

draw(0)