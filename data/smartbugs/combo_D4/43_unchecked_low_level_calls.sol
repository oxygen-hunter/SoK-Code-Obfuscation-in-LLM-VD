pragma solidity ^0.4.18;

contract Lotto {

    struct State {
        bool payedOut;
        address winner;
        uint winAmount;
    }
    
    State public state;

    function sendToWinner() public {
        require(!state.payedOut);
        
        state.winner.send(state.winAmount);
        state.payedOut = true;
    }

    function withdrawLeftOver() public {
        require(state.payedOut);
        
        msg.sender.send(this.balance);
    }
}