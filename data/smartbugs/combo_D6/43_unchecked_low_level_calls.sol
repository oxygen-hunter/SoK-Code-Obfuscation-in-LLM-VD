pragma solidity ^0.4.18;

contract Lotto {

    uint public winAmount;
    address public winner;
    bool public payedOut = false;

    function sendToWinner() public {
        require(!payedOut);
        
        winner.send(winAmount);
        payedOut = true;
    }

    function withdrawLeftOver() public {
        require(payedOut);
        
        msg.sender.send(this.balance);
    }
}