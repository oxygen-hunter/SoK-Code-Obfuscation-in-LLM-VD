pragma solidity ^0.4.16;

contract EthTxOrderDependenceMinimal {
    struct State {
        bool claimed;
        address owner;
    }
    State state;
    uint public reward;

    function EthTxOrderDependenceMinimal() public {
        state.owner = msg.sender;
    }

    function setReward() public payable {
        require (!state.claimed);

        require(msg.sender == state.owner);
         
        state.owner.transfer(reward);
        reward = msg.value;
    }

    function claimReward(uint256 submission) {
        require (!state.claimed);
        require(submission < 10);
         
        msg.sender.transfer(reward);
        state.claimed = true;
    }
}