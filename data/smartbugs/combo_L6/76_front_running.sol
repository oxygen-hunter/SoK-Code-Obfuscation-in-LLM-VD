pragma solidity ^0.4.2;

contract OddsAndEvens {

    struct Player {
        address addr;
        uint number;
    }

    Player[2] public players;

    uint8 tot;
    address owner;

    function OddsAndEvens() {
        owner = msg.sender;
    }

    function play(uint number) payable {
        if (msg.value != 1 ether) throw;

        players[tot] = Player(msg.sender, number);
        tot++;

        if (tot == 2) processWinner(0, players[0].number + players[1].number);
    }

    function processWinner(uint index, uint n) private {
        if (index == 2) {
            delete players;
            tot = 0;
        } else if (index == 0) {
            if (n % 2 == 0) {
                bool res = players[0].addr.send(1800 finney);
            } else {
                processWinner(1, n);
            }
        } else if (index == 1) {
            bool res = players[1].addr.send(1800 finney);
            processWinner(2, n);
        }
    }

    function getProfit() {
        if (msg.sender != owner) throw;
        bool res = msg.sender.send(this.balance);
    }
}