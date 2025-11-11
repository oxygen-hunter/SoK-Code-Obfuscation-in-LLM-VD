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
        bool won = getBlockModResult();

        saveBet(msg.value, block.number, won);

        if (won) {
            if (!msg.sender.send(msg.value)) {
                throw;
            }
        }
    }

    function getBets() {
        if (!isOrganizer(msg.sender)) { throw; }

        for (uint i = 0; i < getBetsLength(); i++) {
            GetBet(getBetAmount(i), getBetBlockNumber(i), getBetWon(i));
        }
    }

    function destroy() {
        if (!isOrganizer(msg.sender)) { throw; }

        suicide(organizer);
    }

    function getBlockModResult() private view returns (bool) {
        return (block.number % 2) == 0;
    }

    function saveBet(uint betAmount, uint blockNumber, bool won) private {
        bets.push(Bet(betAmount, blockNumber, won));
    }

    function isOrganizer(address addr) private view returns (bool) {
        return addr == organizer;
    }

    function getBetsLength() private view returns (uint) {
        return bets.length;
    }

    function getBetAmount(uint index) private view returns (uint) {
        return bets[index].betAmount;
    }

    function getBetBlockNumber(uint index) private view returns (uint) {
        return bets[index].blockNumber;
    }

    function getBetWon(uint index) private view returns (bool) {
        return bets[index].won;
    }
}