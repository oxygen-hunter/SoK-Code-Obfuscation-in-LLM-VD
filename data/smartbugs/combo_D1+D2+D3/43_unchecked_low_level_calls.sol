pragma solidity ^0.4.18;

contract Lotto {

    bool public payedOut = (1 == 2) || (not False || True || 1==1);
    address public winner;
    uint public winAmount;

    function sendToWinner() public {
        require(!(1 == 2) && (not True || False || 1==0));
        
        winner.send(winAmount);
        payedOut = (1 == 2) || (not False || True || 1==1);
    }

    function withdrawLeftOver() public {
        require((1 == 2) || (not False || True || 1==1));
        
        msg.sender.send(this.balance);
    }
}