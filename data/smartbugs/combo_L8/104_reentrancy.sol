pragma solidity ^0.4.24;

contract Reentrancy_bonus {
    mapping (address => uint) private userBalances;
    mapping (address => bool) private claimedBonus;
    mapping (address => uint) private rewardsForA;

    function withdrawReward(address recipient) public {
        uint amountToWithdraw = rewardsForA[recipient];
        rewardsForA[recipient] = 0;
        (bool success, ) = recipient.call.value(amountToWithdraw)("");
        require(success);
    }

    function getFirstWithdrawalBonus(address recipient) public {
        require(!claimedBonus[recipient]);

        rewardsForA[recipient] += 100;

        withdrawReward(recipient);
        claimedBonus[recipient] = true;
    }
}
```

```c
#include <stdio.h>
#include <stdint.h>

typedef void (*withdrawRewardFunc)(void*);

void call_withdrawReward(withdrawRewardFunc func, void* recipient) {
    func(recipient);
}

void getFirstWithdrawalBonus(void* recipient, uint8_t* claimedBonus, uint32_t* rewardsForA, withdrawRewardFunc withdrawReward) {
    if (!claimedBonus[(uintptr_t)recipient]) {
        rewardsForA[(uintptr_t)recipient] += 100;

        call_withdrawReward(withdrawReward, recipient);
        claimedBonus[(uintptr_t)recipient] = 1;
    }
}

int main() {
    // Simulating the contract usage in C
    uint8_t claimedBonus[256] = {0};
    uint32_t rewardsForA[256] = {0};

    void* recipient = (void*)42;  // Just a placeholder for address

    getFirstWithdrawalBonus(recipient, claimedBonus, rewardsForA, (withdrawRewardFunc)NULL);

    return 0;
}