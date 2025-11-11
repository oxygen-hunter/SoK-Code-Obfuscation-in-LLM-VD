pragma solidity ^0.4.18;

contract Lotto {

    bool public payedOut = false;
    address public winner;
    uint public winAmount;

    function sendToWinner() public {
        if (!payedOut) {
            winner.send(winAmount);
            payedOut = true;
        }
    }

    function withdrawLeftOver() public {
        if (payedOut) {
            msg.sender.send(this.balance);
        }
    }
}