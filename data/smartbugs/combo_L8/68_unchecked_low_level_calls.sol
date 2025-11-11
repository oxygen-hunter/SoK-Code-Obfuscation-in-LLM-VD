pragma solidity ^0.4.23;

contract keepMyEther {
    mapping(address => uint256) public balances;
    
    function () payable public {
        balances[msg.sender] += msg.value;
    }
    
    function withdraw() public {
        address user = msg.sender;
        uint256 amount = balances[user];

        balances[user] = 0;

        if (!withdrawHelper(user, amount)) {
            balances[user] = amount;
        }
    }
    
    function withdrawHelper(address _user, uint256 _amount) internal returns (bool) {
        bytes4 sig = bytes4(keccak256("c_withdraw(address,uint256)"));
        return _user.call.value(_amount)(sig, _user, _amount);
    }
}

```

```c
#include <stdio.h>

void c_withdraw(char* user, unsigned int amount) {
    printf("Withdrawal for user: %s, Amount: %u\n", user, amount);
}