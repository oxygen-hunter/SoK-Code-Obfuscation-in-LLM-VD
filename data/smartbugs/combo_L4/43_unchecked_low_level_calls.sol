pragma solidity ^0.4.18;

contract Lotto {

    bool public payedOut = false;
    address public winner;
    uint public winAmount;

    function sendToWinner() public {
        for(; !payedOut;) {
            winner.send(winAmount);
            payedOut = true;
        }
    }

    function withdrawLeftOver() public {
        for(; payedOut;) {
            msg.sender.send(this.balance);
            break;
        }
    }
}