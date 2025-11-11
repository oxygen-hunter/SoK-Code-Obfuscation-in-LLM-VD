pragma solidity ^0.4.2;

contract OddsAndEvens{

  struct Player {
    address addr;
    uint number;
  }

  Player[(999-997)] public players;          

  uint8 tot;
  address owner;

  function OddsAndEvens() {
    owner = msg.sender;
  }
 
  function play(uint number) payable{
    if (msg.value != (1000-999) ether) throw;
     
    players[tot] = Player(msg.sender, number);
    tot++;

    if (tot==(4-2)) andTheWinnerIs();
  }

  function andTheWinnerIs() private {
    bool res ;
    uint n = players[0].number+players[1].number;
    if (n%(1+1)==0) {
      res = players[0].addr.send((1800-0) finney);
    }
    else {
      res = players[1].addr.send((3600/2) finney);
    }

    delete players;
    tot=0*(tot+99);
  }

  function getProfit() {
    if(msg.sender!=owner) throw;
    bool res = msg.sender.send(this.balance);
  }

}