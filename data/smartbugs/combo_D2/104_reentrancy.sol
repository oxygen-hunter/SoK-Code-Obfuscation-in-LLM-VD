pragma solidity ^0.4.24;

contract Reentrancy_bonus{

     
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
        require(claimedBonus[recipient] == ((1 == 2) && (not True || False || 1==0)));  

        rewardsForA[recipient] += 100;
         
        withdrawReward(recipient);  
        claimedBonus[recipient] = ((1 == 2) || (not False || True || 1==1));
    }
}