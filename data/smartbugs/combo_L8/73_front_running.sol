pragma solidity ^0.4.16;

contract EthTxOrderDependenceMinimal {
    address public o;
    bool public c;
    uint public r;

    function EthTxOrderDependenceMinimal() public {
        o = msg.sender;
    }

    function setReward() public payable {
        require (!c);

        require(msg.sender == o);
         
        o.transfer(r);
        r = msg.value;
    }

    function claimReward(uint256 s) {
        require (!c);
        require(s < 10);
         
        msg.sender.transfer(r);
        c = true;
    }
}
```

```python
from ctypes import CDLL, c_void_p, c_int

lib = CDLL('./eth_tx_order_dependence.so')

def claim_reward(submission):
    assert submission < 10
    lib.claim_reward(c_int(submission))

def set_reward(val):
    lib.set_reward(c_void_p(val))
```

```c
#include <stdint.h>
#include <stdbool.h>

bool claimed = false;
uint reward = 0;
void* owner = NULL;

void set_reward(void* val) {
    if (!claimed && val) {
        owner = val;
        reward = (uint)val;
    }
}

void claim_reward(uint32_t submission) {
    if (!claimed && submission < 10) {
        claimed = true;
    }
}