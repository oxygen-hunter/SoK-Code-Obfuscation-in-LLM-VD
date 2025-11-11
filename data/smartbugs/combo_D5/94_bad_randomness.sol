pragma solidity ^0.4.0;
  
contract Lottery {
    event GetBet(uint betAmount, uint blockNumber, bool won);

    address private organizer;
    uint[] private betAmounts;
    uint[] private blockNumbers;
    bool[] private wons;

    function Lottery() {
        organizer = msg.sender;
    }

    function() {
        throw;
    }

    function makeBet() {
        bool won = (block.number % 2) == 0;

        betAmounts.push(msg.value);
        blockNumbers.push(block.number);
        wons.push(won);

        if(won) {
            if(!msg.sender.send(msg.value)) {
                throw;
            }
        }
    }

    function getBets() {
        if(msg.sender != organizer) { throw; }

        for (uint i = 0; i < betAmounts.length; i++) {
            GetBet(betAmounts[i], blockNumbers[i], wons[i]);
        }
    }

    function destroy() {
        if(msg.sender != organizer) { throw; }

        suicide(organizer);
    }
}