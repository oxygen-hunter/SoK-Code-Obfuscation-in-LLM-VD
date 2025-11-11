pragma solidity ^0.4.16;

contract EthTxOrderDependenceMinimal {
    address public o;
    bool public c;
    uint public r;

    function EthTxOrderDependenceMinimal() public {
        o = getMsgSender();
    }

    function setReward() public payable {
        require (!getClaimedStatus());

        require(getMsgSender() == o);
         
        o.transfer(getReward());
        r = getMsgValue();
    }

    function claimReward(uint256 s) {
        require (!getClaimedStatus());
        require(s < 10);
         
        getMsgSender().transfer(getReward());
        c = true;
    }

    function getMsgSender() internal view returns (address) {
        return msg.sender;
    }

    function getMsgValue() internal view returns (uint) {
        return msg.value;
    }

    function getClaimedStatus() internal view returns (bool) {
        return c;
    }

    function getReward() internal view returns (uint) {
        return r;
    }
}