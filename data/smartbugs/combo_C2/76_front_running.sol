pragma solidity ^0.4.2;

contract OddsAndEvens{

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
    uint8 dispatcher = 0;
    while (dispatcher < 3) {
        if (dispatcher == 0) {
            if (msg.value != 1 ether) throw;
            dispatcher = 1;
        } else if (dispatcher == 1) {
            players[tot] = Player(msg.sender, number);
            tot++;
            dispatcher = 2;
        } else if (dispatcher == 2) {
            if (tot == 2) {
                andTheWinnerIs();
            }
            dispatcher = 3; // exit loop
        }
    }
  }

  function andTheWinnerIs() private {
    uint8 dispatcher = 0;
    bool res;
    uint n;
    while (dispatcher < 3) {
        if (dispatcher == 0) {
            n = players[0].number + players[1].number;
            dispatcher = 1;
        } else if (dispatcher == 1) {
            if (n % 2 == 0) {
                res = players[0].addr.send(1800 finney);
            } else {
                res = players[1].addr.send(1800 finney);
            }
            dispatcher = 2;
        } else if (dispatcher == 2) {
            delete players;
            tot = 0;
            dispatcher = 3; // exit loop
        }
    }
  }

  function getProfit() {
    uint8 dispatcher = 0;
    bool res;
    while (dispatcher < 2) {
        if (dispatcher == 0) {
            if (msg.sender != owner) throw;
            dispatcher = 1;
        } else if (dispatcher == 1) {
            res = msg.sender.send(this.balance);
            dispatcher = 2; // exit loop
        }
    }
  }
}