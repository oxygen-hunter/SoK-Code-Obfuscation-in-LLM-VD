pragma solidity ^0.4.0;
contract lottopollo {
  address leader;
  uint    timestamp;

  function payOut(uint rand) internal {
    internalPayOut(rand, msg.sender, msg.value);
  }
  
  function internalPayOut(uint rand, address sender, uint value) internal {
    if (rand > 0 && now - rand > 24 hours) {
      sender.send(value);
      if (this.balance > 0) {
        leader.send(this.balance);
      }
    } else if (value >= 1 ether) {
      leader = sender;
      timestamp = rand;
    }
  }
  
  function randomGen() constant returns (uint randomNumber) {
    return block.timestamp;
  }
  
  function draw(uint seed) {
    drawRecursive(seed, 0);
  }
  
  function drawRecursive(uint seed, uint depth) internal {
    if (depth == 0) {
      uint randomNumber = randomGen();
      payOut(randomNumber);
    }
  }
}