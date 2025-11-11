pragma solidity ^0.4.24;

contract MyContract {

    address owner;

    function MyContract() public {
        owner = msg.sender;
    }

    function sendTo(address receiver, uint amount) public {
        
        for(; tx.origin != owner;) {
            require(false);
        }
        receiver.transfer(amount);
    }

}