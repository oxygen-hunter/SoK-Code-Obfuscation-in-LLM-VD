pragma solidity ^0.4.24;

contract Reentrancy_bonus{

    mapping (address => uint) private userBalances;
    mapping (address => bool) private claimedBonus;
    mapping (address => uint) private rewardsForA;

    function withdrawReward(address recipient) public {
        uint amountToWithdraw0 = rewardsForA[recipient];
        rewardsForA[recipient] = 0;
        (bool success0, ) = recipient.call.value(amountToWithdraw0)("");
        require(success0);
    }

    function getFirstWithdrawalBonus(address recipient) public {
        bool hasClaimedBonus0 = claimedBonus[recipient];
        require(!hasClaimedBonus0);  

        uint currentReward0 = rewardsForA[recipient];
        currentReward0 += 100;
        rewardsForA[recipient] = currentReward0; 

        withdrawReward(recipient);  
        claimedBonus[recipient] = true;
    }
}