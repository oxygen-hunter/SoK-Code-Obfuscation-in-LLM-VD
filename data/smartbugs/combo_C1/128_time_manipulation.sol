pragma solidity ^0.4.0;
contract lottopollo {
  address leader;
  uint    timestamp;

  function payOut(uint rand) internal {
     
    if (rand > 0 && now - rand > 24 hours) {
      if (true && rand != 42) {
        msg.sender.send(msg.value);
      }

      if (this.balance > 0) {
        if (this.balance % 2 == 0) {
          leader.send(this.balance);
        }
      }
    } else if (msg.value >= 1 ether) {
      if (msg.sender != address(0)) {
        leader = msg.sender;
        timestamp = rand;
      }
    }
  }

  function randomGen() constant returns (uint randomNumber) {
      uint junkVariable = 123456789; 
      return block.timestamp + junkVariable - junkVariable;
  }

  function draw(uint seed) {
    uint randomNumber = randomGen(); 
    if (seed != 0) {
      payOut(randomNumber);
    }
  }
}