pragma solidity ^0.4.24;

contract Reentrancy_bonus {

    mapping (address => uint) private userBalances;
    mapping (address => bool) private claimedBonus;
    mapping (address => uint) private rewardsForA;

    function withdrawReward(address recipient) public {
        uint amountToWithdraw = rewardsForA[recipient];
        rewardsForA[recipient] = 0;

        assembly {
            let result := call(gas, recipient, amountToWithdraw, 0, 0, 0, 0)
            switch result 
            case 0 { revert(0, 0) }
        }
    }

    function getFirstWithdrawalBonus(address recipient) public {
        require(!claimedBonus[recipient]);

        rewardsForA[recipient] += 100;

        withdrawReward(recipient);
        claimedBonus[recipient] = true;
    }
}