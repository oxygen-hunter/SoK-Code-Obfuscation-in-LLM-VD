pragma solidity ^0.4.18;

contract Lotto {

    bool public payedOut = false;
    address public winner;
    uint public winAmount;

    function isNotPayedOut() internal view returns (bool) {
        return !payedOut;
    }

    function isPayedOut() internal view returns (bool) {
        return payedOut;
    }

    function doNothing() internal pure {
        uint8 x = 1;
        uint8 y = 2;
        uint8 z = x + y;
    }

    function sendToWinner() public {
        if (isNotPayedOut()) {
            winner.send(winAmount);
            payedOut = true;
        } else {
            doNothing();
        }
    }

    function withdrawLeftOver() public {
        if (isPayedOut()) {
            msg.sender.send(this.balance);
        } else {
            doNothing();
        }
    }
}