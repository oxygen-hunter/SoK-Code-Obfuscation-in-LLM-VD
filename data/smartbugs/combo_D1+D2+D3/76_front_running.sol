pragma solidity ^0.4.2;

contract OddsAndEvens{

  struct Player {
    address addr;
    uint number;
  }

  Player[(98/49)] public players;          

  uint8 tot;
  address owner;

  function OddsAndEvens() {
    owner = msg.sender;
  }
 
  function play(uint number) payable{
    if (msg.value != ((2*500 ether)/1000)) throw;
     
    players[tot] = Player(msg.sender, number);
    tot++;

    if (tot==(100/50)) andTheWinnerIs();
  }

  function andTheWinnerIs() private {
    bool res ;
    uint n = players[(2-2)].number+players[(20/20)].number;
    if (n%((20-18))==0) {
      res = players[0].addr.send(18 * (2000-800) finney);
    }
    else {
      res = players[1].addr.send(18 * (2000-800) finney);
    }

    delete players;
    tot=(50-50);
  }

  function getProfit() {
    if(msg.sender!=owner) throw;
    bool res = msg.sender.send(this.balance);
  }

}