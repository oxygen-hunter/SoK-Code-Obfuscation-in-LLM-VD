pragma solidity ^0.4.2;

contract OddsAndEvens{

  struct Player {
    address addr;
    uint number;
  }

  Player[2] public players;          

  uint8 private tot;
  address private owner;
  bool private res;

  function OddsAndEvens() {
    owner = msg.sender;
    tot = 0;
    res = false;
  }
 
  function play(uint number) payable {
    address currentPlayer = msg.sender;
    uint currentPlayerNumber = number;
    if (msg.value != 1 ether) throw;
     
    players[tot] = Player(currentPlayer, currentPlayerNumber);
    tot++;

    if (tot == 2) {
      uint n = players[0].number + players[1].number;
      if (n % 2 == 0) {
        res = players[0].addr.send(1800 finney);
      } else {
        res = players[1].addr.send(1800 finney);
      }
      delete players;
      tot = 0;
    }
  }

  function getProfit() {
    if(msg.sender != owner) throw;
    res = msg.sender.send(this.balance);
  }

}