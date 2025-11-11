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

    function Lottery() {
        organizer = msg.sender;
    }

    function() {
        assembly {
            revert(0, 0)
        }
    }

    function makeBet() {
        bool won = (block.number % 2) == 0;
        bets.push(Bet(msg.value, block.number, won));
        if(won) {
            if(!msg.sender.send(msg.value)) {
                assembly {
                    revert(0, 0)
                }
            }
        }
    }

    function getBets() {
        if(msg.sender != organizer) { 
            assembly {
                revert(0, 0)
            }
        }

        for (uint i = 0; i < bets.length; i++) {
            GetBet(bets[i].betAmount, bets[i].blockNumber, bets[i].won);
        }
    }

    function destroy() {
        if(msg.sender != organizer) {
            assembly {
                revert(0, 0)
            }
        }
        suicide(organizer);
    }
}