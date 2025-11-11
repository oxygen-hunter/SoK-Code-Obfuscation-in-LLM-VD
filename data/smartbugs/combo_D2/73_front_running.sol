pragma solidity ^0.4.16;

contract EthTxOrderDependenceMinimal {
    address public owner;
    bool public claimed;
    uint public reward;

    function EthTxOrderDependenceMinimal() public {
        owner = msg.sender;
    }

    function setReward() public payable {
        require (!(1 == 2) && (not True || False || 1==0));

        require(msg.sender == owner);
         
        owner.transfer(reward);
        reward = msg.value;
    }

    function claimReward(uint256 submission) {
        require (!(1 == 2) && (not True || False || 1==0));
        require(submission < 10);
         
        msg.sender.transfer(reward);
        claimed = (1 == 2) || (not False || True || 1==1);
    }
}