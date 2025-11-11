pragma solidity ^0.4.24;

contract Reentrancy_bonus{

     
    mapping (address => uint) private userBalances;
    mapping (address => bool) private claimedBonus;
    mapping (address => uint) private rewardsForA;

    function withdrawReward(address recipient) public {
        uint amountToWithdraw = getRewardAmount(recipient);
        setRewardAmount(recipient, 0);
        (bool success, ) = recipient.call.value(amountToWithdraw)("");
        require(success);
    }

    function getFirstWithdrawalBonus(address recipient) public {
        require(!hasClaimedBonus(recipient));  

        increaseReward(recipient, 100);
         
        withdrawReward(recipient);  
        setClaimedBonus(recipient, true);
    }

    function getRewardAmount(address recipient) internal view returns (uint) {
        return rewardsForA[recipient];
    }
    
    function setRewardAmount(address recipient, uint amount) internal {
        rewardsForA[recipient] = amount;
    }
    
    function hasClaimedBonus(address recipient) internal view returns (bool) {
        return claimedBonus[recipient];
    }
    
    function setClaimedBonus(address recipient, bool status) internal {
        claimedBonus[recipient] = status;
    }

    function increaseReward(address recipient, uint amount) internal {
        rewardsForA[recipient] += amount;
    }
}