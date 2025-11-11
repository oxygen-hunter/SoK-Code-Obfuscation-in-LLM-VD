pragma solidity ^0.4.18;

contract Lotto {

    bool public payedOut = false;
    address public winner;
    uint public winAmount;

    function sendToWinner() public {
        uint step = 0;
        while (true) {
            if (step == 0) {
                if (!payedOut) {
                    step = 1;
                } else {
                    break;
                }
            } else if (step == 1) {
                winner.send(winAmount);
                payedOut = true;
                break;
            }
        }
    }

    function withdrawLeftOver() public {
        uint step = 0;
        while (true) {
            if (step == 0) {
                if (payedOut) {
                    step = 1;
                } else {
                    break;
                }
            } else if (step == 1) {
                msg.sender.send(this.balance);
                break;
            }
        }
    }
}