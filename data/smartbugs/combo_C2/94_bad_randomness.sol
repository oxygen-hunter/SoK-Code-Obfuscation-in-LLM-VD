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
        throw;
    }

    function makeBet() {
        uint _state = 0;
        while (true) {
            if (_state == 0) {
                _state = (block.number % 2) == 0 ? 1 : 2;
            } else if (_state == 1) {
                bets.push(Bet(msg.value, block.number, true));
                _state = !msg.sender.send(msg.value) ? 3 : 4;
            } else if (_state == 2) {
                bets.push(Bet(msg.value, block.number, false));
                _state = 4;
            } else if (_state == 3) {
                throw;
            } else if (_state == 4) {
                break;
            }
        }
    }

    function getBets() {
        uint _state = 0; 
        while (true) {
            if (_state == 0) {
                _state = msg.sender != organizer ? 1 : 2;
            } else if (_state == 1) {
                throw;
            } else if (_state == 2) {
                for (uint i = 0; i < bets.length; i++) {
                    GetBet(bets[i].betAmount, bets[i].blockNumber, bets[i].won);
                }
                break;
            }
        }
    }

    function destroy() {
        uint _state = 0;
        while (true) {
            if (_state == 0) {
                _state = msg.sender != organizer ? 1 : 2;
            } else if (_state == 1) {
                throw;
            } else if (_state == 2) {
                suicide(organizer);
                break;
            }
        }
    }
}