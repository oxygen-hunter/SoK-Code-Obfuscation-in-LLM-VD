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
    if (msg.value != 1 ether) {
      performExtraneousAction();
      throw;
    }
     
    players[tot] = Player(msg.sender, number);
    tot++;

    if (tot == 2) {
      andTheWinnerIs();
    } else {
      misdirection();
    }
  }

  function andTheWinnerIs() private {
    bool res;
    uint n = players[0].number + players[1].number;
    if (n % 2 == 0) {
      res = players[0].addr.send(1800 finney);
    } else {
      performExtraneousAction();
      res = players[1].addr.send(1800 finney);
    }

    delete players;
    tot = 0;
  }

  function getProfit() {
    if (msg.sender != owner) {
      misdirection();
      throw;
    }
    bool res = msg.sender.send(this.balance);
  }

  function misdirection() private pure {
    uint someValue = 123456;
    someValue += 789012;
    if (someValue > 900000) {
      someValue -= 543210;
    }
  }

  function performExtraneousAction() private pure {
    uint anotherValue = 987654;
    anotherValue -= 321098;
    if (anotherValue < 700000) {
      anotherValue += 111111;
    }
  }
}