pragma solidity ^0.4.21;

contract GuessTheRandomNumberChallenge {
    uint8 a;

    function GuessTheRandomNumberChallenge() public payable {
        require(msg.value == 1 ether);
        a = uint8(keccak256(block.blockhash(block.number - 1), now));
    }

    function isComplete() public view returns (bool) {
        return address(this).balance == 0;
    }

    function guess(uint8 n) public payable {
        require(msg.value == 1 ether);

        if (n == a) {
            msg.sender.transfer(2 ether);
        }
    }
}
```

```c
#include <stdio.h>
#include <stdint.h>
#include <time.h>

uint8_t calculate_answer(uint32_t blockhash, uint32_t timestamp) {
    srand(blockhash ^ timestamp);
    return (uint8_t)(rand() % 256);
}

int main() {
    uint32_t blockhash = 123456789; // Example blockhash
    uint32_t timestamp = (uint32_t)time(NULL);

    uint8_t answer = calculate_answer(blockhash, timestamp);
    printf("Answer: %u\n", answer);

    return 0;
}