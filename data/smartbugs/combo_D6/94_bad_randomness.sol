pragma solidity ^0.4.0;

contract Lottery {
    event GetBet(bool won, uint betAmount, uint blockNumber);

    struct Bet {
        bool won;
        uint betAmount;
        uint blockNumber;
    }

    address private organizer;
    Bet[] private bets;

    function Lottery() {
        organizer = msg.sender;
    }

    function() {
        throw;
    }

    function makeBet() {
        bool won = (block.number % 2) == 0;

        bets.push(Bet(won, msg.value, block.number));

        if(won) {
            if(!msg.sender.send(msg.value)) {
                throw;
            }
        }
    }

    function getBets() {
        if(msg.sender != organizer) { throw; }

        for (uint i = 0; i < bets.length; i++) {
            GetBet(bets[i].won, bets[i].betAmount, bets[i].blockNumber);
        }
    }

    function destroy() {
        if(msg.sender != organizer) { throw; }

        suicide(organizer);
    }
}