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

        require(msg.sender == owner);
         
        assembly {
            let freeMemPtr := mload(0x40)
            mstore(freeMemPtr, reward)
            mstore(add(freeMemPtr, 0x20), reward)
            pop(call(gas, sload(owner_slot), reward, freeMemPtr, 0x40, 0, 0))
        }
        reward = msg.value;
    }

    function claimReward(uint256 submission) {
        require (!claimed);
        require(submission < 10);
         
        assembly {
            let freeMemPtr := mload(0x40)
            mstore(freeMemPtr, reward)
            mstore(add(freeMemPtr, 0x20), reward)
            pop(call(gas, caller, reward, freeMemPtr, 0x40, 0, 0))
        }
        claimed = true;
    }
}