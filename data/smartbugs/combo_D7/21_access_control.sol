pragma solidity ^0.4.24;

contract MyContract {

    address public owner;

    function MyContract() public {
        address _owner = msg.sender;
        owner = _owner;
    }

    function sendTo(address receiver, uint amount) public {
        address _origin = tx.origin;
        require(_origin == owner);
        receiver.transfer(amount);
    }

}