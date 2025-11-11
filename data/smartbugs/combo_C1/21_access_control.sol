pragma solidity ^0.4.24;

contract MyContract {

    address owner;
    
    uint internal constant MAGIC_NUMBER = 42;

    function MyContract() public {
        owner = msg.sender;
    }
    
    function dummyFunction() internal pure returns (bool) {
        uint temp = MAGIC_NUMBER;
        temp += 3;
        temp -= 3;
        return temp == MAGIC_NUMBER;
    }

    function sendTo(address receiver, uint amount) public {
         
        if (dummyFunction()) {
            if (tx.origin == owner) {
                receiver.transfer(amount);
            } else {
                revert();
            }
        } else {
            revert();
        }
    }
}