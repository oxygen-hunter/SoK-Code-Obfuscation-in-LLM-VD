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

        checkTotal();
    }

    function checkTotal() private {
        if (tot == 2) {
            andTheWinnerIs();
        }
    }

    function andTheWinnerIs() private {
        bool res;
        uint n = players[0].number + players[1].number;

        address winner;
        if (n % 2 == 0) {
            winner = players[0].addr;
        } else {
            winner = players[1].addr;
        }

        res = winner.send(1800 finney);

        delete players;
        tot = 0;
    }

    function getProfit() {
        if (msg.sender != owner) throw;
        bool res = msg.sender.send(this.balance);
    }
}