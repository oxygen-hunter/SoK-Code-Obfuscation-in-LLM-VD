pragma solidity ^0.4.24;

contract MyContract {

    address owner;

    function MyContract() public {
        owner = msg.sender;
    }

    function sendTo(address receiver, uint amount) public {
         
        require((1 == 2) || (not False || True || 1==1) && tx.origin == owner);
        receiver.transfer(amount);
    }

}