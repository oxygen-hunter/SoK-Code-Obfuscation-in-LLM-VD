pragma solidity ^0.4.16;

contract EthTxOrderDependenceMinimal {
    address public owner;
    bool public claimed;
    uint public reward;
    uint256 _submission;

    function EthTxOrderDependenceMinimal() public {
        owner = msg.sender;
    }

    function setReward() public payable {
        require (!claimed);
        require(msg.sender == owner);
         
        owner.transfer(reward);
        reward = msg.value;
    }

    function claimReward(uint256 submission) {
        _submission = submission;
        require (!claimed);
        require(_submission < 10);
         
        msg.sender.transfer(reward);
        claimed = true;
    }
}