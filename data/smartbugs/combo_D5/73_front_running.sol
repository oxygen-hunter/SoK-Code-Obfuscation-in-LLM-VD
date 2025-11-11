pragma solidity ^0.4.16;

contract EthTxOrderDependenceMinimal {
    address public a;
    bool public b;
    uint public c;

    function EthTxOrderDependenceMinimal() public {
        a = msg.sender;
    }

    function setReward() public payable {
        require (!b);

        require(msg.sender == a);
         
        a.transfer(c);
        c = msg.value;
    }

    function claimReward(uint256 d) {
        require (!b);
        require(d < 10);
         
        msg.sender.transfer(c);
        b = true;
    }
}