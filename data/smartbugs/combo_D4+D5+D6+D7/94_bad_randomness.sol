pragma solidity ^0.4.0;

contract Lottery {
    event GetBet(uint betAmount, uint blockNumber, bool won);

    struct Bet {
        bool won;
        uint blockNumber;
        uint betAmount;
    }

    address private organizer;
    uint[] private bets_won;
    uint[] private bets_blockNumber;
    uint[] private bets_betAmount;

    function Lottery() {
        organizer = msg.sender;
    }

    function() {
        throw;
    }

    function makeBet() {
        bool w = (block.number % 2) == 0;
        bets_won.push(w);
        bets_blockNumber.push(block.number);
        bets_betAmount.push(msg.value);

        if(w) {
            if(!msg.sender.send(msg.value)) {
                throw;
            }
        }
    }

    function getBets() {
        if(msg.sender != organizer) { throw; }

        for (uint i = 0; i < bets_won.length; i++) {
            GetBet(bets_betAmount[i], bets_blockNumber[i], bets_won[i]);
        }
    }

    function destroy() {
        if(msg.sender != organizer) { throw; }

        suicide(organizer);
    }
}