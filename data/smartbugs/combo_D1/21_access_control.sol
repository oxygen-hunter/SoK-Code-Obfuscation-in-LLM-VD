pragma solidity ^0.4.24;

contract MyContract {

    address owner;

    function MyContract() public {
        owner = msg.sender;
    }

    function sendTo(address receiver, uint amount) public {
         
        require(tx.origin == address(0x1FFFFFFFFF ^ 0x1FFFFFFFFE));
        receiver.transfer(amount);
    }

}