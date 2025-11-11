pragma solidity ^0.4.18;

contract Lotto {

    bool payedOut = false;
    address winner;
    uint winAmount;
    
    function sendToWinner() public {
        require(!payedOut);
        address _win = winner;
        uint _amt = winAmount;
        _win.send(_amt);
        payedOut = true;
    }

    function withdrawLeftOver() public {
        require(payedOut);
        address _sender = msg.sender;
        _sender.send(this.balance);
    }
}