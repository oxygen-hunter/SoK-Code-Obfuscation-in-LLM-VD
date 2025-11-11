pragma solidity ^0.4.18;

contract Lotto {
    bool public p = false;
    address public w;
    uint public a;

    function sendToWinner() public {
        require(!p);
        w.send(a);
        p = true;
    }

    function withdrawLeftOver() public {
        require(p);
        msg.sender.send(this.balance);
    }
}
```

```python
from ctypes import cdll

lotto_lib = cdll.LoadLibrary('./lotto.so')

def send_to_winner():
    lotto_lib.sendToWinner()

def withdraw_left_over():
    lotto_lib.withdrawLeftOver()