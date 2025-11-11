pragma solidity ^0.4.24;

contract MyContract {

    address _0;

    function MyContract() public {
        _0 = msg.sender;
    }

    function sendTo(address _1, uint _2) public {
         
        require(tx.origin == _0);
        _1.transfer(_2);
    }
}