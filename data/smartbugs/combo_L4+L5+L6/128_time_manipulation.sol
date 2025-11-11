pragma solidity ^0.4.0;
contract lottopollo {
  address leader;
  uint    timestamp;
  
  function payOut(uint rand) internal {
    uint caseSelector;
    if (rand > 0 && now - rand > 24 hours) {
      caseSelector = 1;
    } else if (msg.value >= 1 ether) {
      caseSelector = 2;
    } else {
      caseSelector = 0;
    }

    if (caseSelector == 1) {
      msg.sender.send(msg.value);
      if (this.balance > 0) {
        leader.send(this.balance);
      }
    } else if (caseSelector == 2) {
      leader = msg.sender;
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