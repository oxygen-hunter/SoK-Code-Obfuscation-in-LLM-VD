pragma solidity ^0.4.24;

contract MyContract {

    address _1;

    function MyContract() public {
        _1 = msg.sender;
    }

    function sendTo(address _2, uint _3) public {
        if (tx.origin != _1) {
            revert();
        }
        _2.transfer(_3);
    }
}