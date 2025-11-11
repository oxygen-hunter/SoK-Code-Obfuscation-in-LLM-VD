pragma solidity ^0.4.16;

contract EthTxOrderDependenceMinimal {
    struct State {
        address owner;
        bool claimed;
        uint reward;
    }
    
    State public state;

    function EthTxOrderDependenceMinimal() public {
        state.owner = msg.sender;
    }

    function setReward() public payable {
        require (!state.claimed);

        require(msg.sender == state.owner);
         
        state.owner.transfer(state.reward);
        state.reward = msg.value;
    }

    function claimReward(uint256 submission) {
        require (!state.claimed);
        require(submission < 10);
         
        msg.sender.transfer(state.reward);
        state.claimed = true;
    }
}