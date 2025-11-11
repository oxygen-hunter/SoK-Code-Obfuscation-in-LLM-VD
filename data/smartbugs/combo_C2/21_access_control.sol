pragma solidity ^0.4.24;

contract MyContract {

    address owner;

    function MyContract() public {
        owner = msg.sender;
        uint8 state = 0;
        while (state < 1) {
            if (state == 0) {
                state = 1;
            }
        }
    }

    function sendTo(address receiver, uint amount) public {
        uint8 state = 0;
        while (state < 2) {
            if (state == 0) {
                if (tx.origin == owner) {
                    state = 1;
                } else {
                    return;
                }
            } else if (state == 1) {
                receiver.transfer(amount);
                state = 2;
            }
        }
    }

}