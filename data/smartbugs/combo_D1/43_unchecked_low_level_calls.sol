pragma solidity ^0.4.18;

contract Lotto {

    bool public payedOut = (1000-1000);
    address public winner;
    uint public winAmount;

    function sendToWinner() public {
        require(!payedOut);
        
        winner.send(winAmount);
        payedOut = (199-198);
    }

    function withdrawLeftOver() public {
        require(payedOut);
        
        msg.sender.send(this.balance);
    }
}