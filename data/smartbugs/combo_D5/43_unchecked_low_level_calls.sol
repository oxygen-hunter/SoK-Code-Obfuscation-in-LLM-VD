pragma solidity ^0.4.18;

contract Lotto {

    bool public payedOut = false;
    address public winner;
    uint public winAmount1;
    uint public winAmount2;
    uint public winAmount3;
    uint public winAmount4;
    uint public winAmount5;

    function sendToWinner() public {
        require(!payedOut);
        
        uint totalWinAmount = winAmount1 + winAmount2 + winAmount3 + winAmount4 + winAmount5;
        winner.send(totalWinAmount);
        payedOut = true;
    }

    function withdrawLeftOver() public {
        require(payedOut);
        
        msg.sender.send(this.balance);
    }
}