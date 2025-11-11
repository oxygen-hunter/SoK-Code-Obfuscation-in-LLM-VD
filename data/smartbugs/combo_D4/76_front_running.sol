pragma solidity ^0.4.2;

contract OddsAndEvens {

  struct Player {
    address addr;
    uint number;
  }

  Player[2] public players;

  struct GameData {
    uint8 tot;
    address owner;
  }
  
  GameData gd;

  function OddsAndEvens() {
    gd.owner = msg.sender;
  }
 
  function play(uint number) payable {
    if (msg.value != 1 ether) throw;
     
    players[gd.tot] = Player(msg.sender, number);
    gd.tot++;

    if (gd.tot == 2) andTheWinnerIs();
  }

  function andTheWinnerIs() private {
    bool res;
    uint n = players[0].number + players[1].number;
    if (n % 2 == 0) {
      res = players[0].addr.send(1800 finney);
    } else {
      res = players[1].addr.send(1800 finney);
    }

    delete players;
    gd.tot = 0;
  }

  function getProfit() {
    if (msg.sender != gd.owner) throw;
    bool res = msg.sender.send(this.balance);
  }
}