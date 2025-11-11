```solidity
pragma solidity ^0.4.15;

contract EtherLotto {
    uint constant TICKET_AMOUNT = 10;
    uint constant FEE_AMOUNT = 1;
    address public bank;
    uint public pot;

    function EtherLotto() {
        bank = msg.sender;
    }

    function play() payable {
        assert(msg.value == TICKET_AMOUNT);
        pot += msg.value;

        uint random = getRandom() % 2;

        if (random == 0) {
            bank.transfer(FEE_AMOUNT);
            msg.sender.transfer(pot - FEE_AMOUNT);
            pot = 0;
        }
    }

    function getRandom() internal returns (uint) {
        return uint(sha3(block.timestamp));
    }
}
```

```python
import ctypes
import os

# Load C DLL for random number generation
def get_random():
    lib_path = os.path.abspath("librandom.so")  # Assume the C library is compiled as librandom.so
    c_lib = ctypes.CDLL(lib_path)
    return c_lib.generate_random()

# The C code for 'generate_random' should be implemented as follows:
# #include <stdlib.h>
# #include <time.h>
#
# int generate_random() {
#     srand(time(NULL));
#     return rand();
# }

# Python wrapper function
def play_game():
    random_number = get_random() % 2
    return random_number

# Example of calling the play_game function
if __name__ == "__main__":
    result = play_game()
    print(f"Random number generated: {result}")
```

(Note: The `librandom.so` library should be compiled from the C code snippet provided in the comment using a C compiler.)