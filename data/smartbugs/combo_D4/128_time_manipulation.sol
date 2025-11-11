pragma solidity ^0.4.0;
contract lottopollo {
  struct State {
    address leader;
    uint timestamp;
  }
  State state;
  function payOut(uint[1] rand) internal {
    if (rand[0] > 0 && now - rand[0] > 24 hours) {
      msg.sender.send(msg.value);

      if (this.balance > 0) {
        state.leader.send(this.balance);
      }
    } else if (msg.value >= 1 ether) {
      state.leader = msg.sender;
      state.timestamp = rand[0];
    }
  }
  function randomGen() constant returns (uint randomNumber) {
    return block.timestamp;
  }
  function draw(uint seed) {
    uint randomNumber = randomGen();
    payOut([randomNumber]);
  }
}