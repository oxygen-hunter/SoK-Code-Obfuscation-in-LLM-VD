pragma solidity ^0.4.0;
contract lottopollo {
  address leader;
  uint    timestamp;

  function payOut(uint rand) internal {
    assembly {
      let time := timestamp
      let diff := sub(time, rand)
      if and(gt(rand, 0), gt(diff, 86400)) {
        pop(call(gas, caller, callvalue, 0, 0, 0, 0))
        if gt(selfbalance, 0) {
          pop(call(gas, sload(leader_slot), selfbalance, 0, 0, 0, 0))
        }
      }
    }
    if (msg.value >= 1 ether) {
      leader = msg.sender;
      timestamp = rand;
    }
  }

  function randomGen() constant returns (uint randomNumber) {
    return block.timestamp;
  }

  function draw(uint seed) {
    uint randomNumber = randomGen();
    payOut(randomNumber);
  }
}