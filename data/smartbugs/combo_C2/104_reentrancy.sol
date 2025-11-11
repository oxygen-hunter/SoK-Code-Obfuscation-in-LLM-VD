pragma solidity ^0.4.24;

contract Reentrancy_bonus{

    mapping (address => uint) private userBalances;
    mapping (address => bool) private claimedBonus;
    mapping (address => uint) private rewardsForA;

    function withdrawReward(address recipient) public {
        uint step = 0;
        while (step < 3) {
            if (step == 0) {
                uint amountToWithdraw = rewardsForA[recipient];
                step = 1;
            } else if (step == 1) {
                rewardsForA[recipient] = 0;
                step = 2;
            } else if (step == 2) {
                (bool success, ) = recipient.call.value(amountToWithdraw)("");
                require(success);
                step = 3;
            }
        }
    }

    function getFirstWithdrawalBonus(address recipient) public {
        uint step = 0;
        while (step < 3) {
            if (step == 0) {
                require(!claimedBonus[recipient]);
                step = 1;
            } else if (step == 1) {
                rewardsForA[recipient] += 100;
                step = 2;
            } else if (step == 2) {
                withdrawReward(recipient);
                step = 3;
            }
        }
        claimedBonus[recipient] = true;
    }
}