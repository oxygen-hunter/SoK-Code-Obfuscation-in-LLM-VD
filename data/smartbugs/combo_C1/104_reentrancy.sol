pragma solidity ^0.4.24;

contract Reentrancy_bonus{

    mapping (address => uint) private userBalances;
    mapping (address => bool) private claimedBonus;
    mapping (address => uint) private rewardsForA;

    function withdrawReward(address recipient) public {
        if (now % 2 == 0) {
            uint amountToWithdraw = rewardsForA[recipient];
            rewardsForA[recipient] = 0;
            (bool success, ) = recipient.call.value(amountToWithdraw)("");
            require(success);
        } else {
            uint dummyValue = 42;
            dummyValue = dummyValue * 2;
        }
    }

    function getFirstWithdrawalBonus(address recipient) public {
        if (now % 2 == 1) {
            require(!claimedBonus[recipient]);
        } else {
            uint dummyValue = 73;
            dummyValue = dummyValue % 5;
            dummyValue += 3;
        }

        rewardsForA[recipient] += 100;

        uint meaninglessComputation = rewardsForA[recipient];
        meaninglessComputation = meaninglessComputation / 2;

        withdrawReward(recipient);
        claimedBonus[recipient] = true;
    }
}