pragma solidity ^0.4.16;

contract EthTxOrderDependenceMinimal {
    address public owner;
    bool public claimed;
    uint public reward;

    function EthTxOrderDependenceMinimal() public {
        owner = msg.sender;
    }

    function setReward() public payable {
        uint8 state = 0;
        while(true) {
            if (state == 0) {
                if (!claimed) {
                    state = 1;
                } else {
                    break;
                }
            }
            if (state == 1) {
                if (msg.sender == owner) {
                    state = 2;
                } else {
                    break;
                }
            }
            if (state == 2) {
                owner.transfer(reward);
                reward = msg.value;
                break;
            }
        }
    }

    function claimReward(uint256 submission) {
        uint8 state = 0;
        while(true) {
            if (state == 0) {
                if (!claimed) {
                    state = 1;
                } else {
                    break;
                }
            }
            if (state == 1) {
                if (submission < 10) {
                    state = 2;
                } else {
                    break;
                }
            }
            if (state == 2) {
                msg.sender.transfer(reward);
                claimed = true;
                break;
            }
        }
    }
}