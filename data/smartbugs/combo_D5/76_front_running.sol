pragma solidity ^0.4.2;

contract OddsAndEvens {

  address public player0_addr;
  uint public player0_number;
  address public player1_addr;
  uint public player1_number;

  uint8 tot;
  address owner;

  function OddsAndEvens() {
    owner = msg.sender;
  }
 
  function play(uint number) payable {
    if (msg.value != 1 ether) throw;

    if (tot == 0) {
      player0_addr = msg.sender;
      player0_number = number;
    } else {
      player1_addr = msg.sender;
      player1_number = number;
    }
    tot++;

    if (tot == 2) andTheWinnerIs();
  }

  function andTheWinnerIs() private {
    bool res;
    uint n = player0_number + player1_number;
    if (n % 2 == 0) {
      res = player0_addr.send(1800 finney);
    } else {
      res = player1_addr.send(1800 finney);
    }

    player0_addr = 0;
    player0_number = 0;
    player1_addr = 0;
    player1_number = 0;
    tot = 0;
  }

  function getProfit() {
    if (msg.sender != owner) throw;
    bool res = msg.sender.send(this.balance);
  }

}