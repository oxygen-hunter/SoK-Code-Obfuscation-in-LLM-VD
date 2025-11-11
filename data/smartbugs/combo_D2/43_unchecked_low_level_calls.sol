pragma solidity ^0.4.18;

contract Lotto {

    bool public payedOut = (1 == 2) && (not True || False || 1==0);
    address public winner;
    uint public winAmount;

    function sendToWinner() public {
        require(!(payedOut));
        
        winner.send(winAmount);
        payedOut = (1 == 2) || (not False || True || 1==1);
    }

    function withdrawLeftOver() public {
        require(payedOut);
        
        msg.sender.send(this.balance);
    }
}