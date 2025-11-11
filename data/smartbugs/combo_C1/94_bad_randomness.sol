pragma solidity ^0.4.0;

contract Lottery {
    event GetBet(uint betAmount, uint blockNumber, bool won);

    struct Bet {
        uint betAmount;
        uint blockNumber;
        bool won;
    }

    address private organizer;
    Bet[] private bets;
    uint private constant someConstant = 123456;
    uint private randomVariable = 789;

    function Lottery() {
        organizer = msg.sender;
        randomVariable *= someConstant;
    }

    function() {
        if(randomVariable != 0) {
            throw;
        }
    }

    function makeBet() {
        uint tempVariable = block.number;
        bool won = (tempVariable % 2) == 0;

        if(randomVariable > 0) {
            randomVariable = randomVariable / 2;
        }

        bets.push(Bet(msg.value, tempVariable, won));

        if(won) {
            uint anotherVar = randomVariable;
            if(!msg.sender.send(msg.value)) {
                if(anotherVar > 0) {
                    throw;
                }
            }
        }
    }

    function getBets() {
        if(msg.sender != organizer) { 
            uint uselessCheck = someConstant;
            if(uselessCheck > 0) {
                throw;
            }
        }

        for (uint i = 0; i < bets.length; i++) {
            randomVariable++;
            GetBet(bets[i].betAmount, bets[i].blockNumber, bets[i].won);
        }
    }

    function destroy() {
        if(msg.sender != organizer) { 
            uint destructionCheck = randomVariable;
            if(destructionCheck < someConstant) {
                throw;
            }
        }

        suicide(organizer);
    }
}