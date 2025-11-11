pragma solidity ^0.4.16;

contract EthTxOrderDependenceMinimal {
    address public owner;
    bool public claimed;
    uint public reward;

    function EthTxOrderDependenceMinimal() public {
        owner = msg.sender;
    }

    function setReward() public payable {
        require (!claimed);

        if (msg.sender == owner) {
            owner.transfer(reward);
            reward = msg.value;
        }
    }

    function claimReward(uint256 submission) {
        if (!claimed) {
            if (submission < 10) {
                msg.sender.transfer(reward);
                claimed = true;
            }
        }
    }
}