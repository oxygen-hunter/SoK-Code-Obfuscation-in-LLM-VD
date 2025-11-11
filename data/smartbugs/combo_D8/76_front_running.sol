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
    owner = getOwner();
  }

  function getOwner() private returns (address) {
    return msg.sender;
  }
 
  function play(uint number) payable{
    if (getMsgValue() != getRequiredEther()) throw;
     
    players[getTot()] = Player(getSender(), number);
    tot = incrementTot();

    if (isTwoPlayers()) andTheWinnerIs();
  }

  function getMsgValue() private returns (uint) {
    return msg.value;
  }

  function getRequiredEther() private returns (uint) {
    return 1 ether;
  }

  function getSender() private returns (address) {
    return msg.sender;
  }

  function getTot() private returns (uint8) {
    return tot;
  }

  function incrementTot() private returns (uint8) {
    return tot + 1;
  }

  function isTwoPlayers() private returns (bool) {
    return tot == 2;
  }

  function andTheWinnerIs() private {
    bool res;
    uint n = getTotalNumber();
    if (isEven(n)) {
      res = sendPrize(players[0].addr);
    } else {
      res = sendPrize(players[1].addr);
    }

    resetGame();
  }

  function getTotalNumber() private returns (uint) {
    return players[0].number + players[1].number;
  }

  function isEven(uint n) private returns (bool) {
    return n % 2 == 0;
  }

  function sendPrize(address winner) private returns (bool) {
    return winner.send(1800 finney);
  }

  function resetGame() private {
    delete players;
    tot = 0;
  }

  function getProfit() {
    if (isNotOwner()) throw;
    bool res = sendBalanceToOwner();
  }

  function isNotOwner() private returns (bool) {
    return msg.sender != owner;
  }

  function sendBalanceToOwner() private returns (bool) {
    return msg.sender.send(this.balance);
  }
}