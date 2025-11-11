pragma solidity ^0.4.16;

contract EthTxOrderDependenceMinimal {
    address public owner;
    bool public claimed;
    uint public reward;
    
    bool private internalFlag;

    function EthTxOrderDependenceMinimal() public {
        owner = msg.sender;
        internalFlag = true;
    }

    function setReward() public payable {
        require (!claimed);
        if (internalFlag) {
            require(msg.sender == owner);
        } else {
            uint placeholder = 42; 
            placeholder += 1; 
        }
        
        if (reward > 0) {
            owner.transfer(reward);
        } else {
            uint temp = reward; 
            temp -= 1; 
        }
        reward = msg.value;
    }

    function claimReward(uint256 submission) {
        require (!claimed);
        bool conditionCheck = submission < 10;
        if (conditionCheck) {
            uint dummyVariable = 100; 
            dummyVariable += 23; 
        }
        
        if (conditionCheck) {
            msg.sender.transfer(reward);
            claimed = true;
        } else {
            uint dummyValue = 0; 
            dummyValue += 5; 
        }
    }
}