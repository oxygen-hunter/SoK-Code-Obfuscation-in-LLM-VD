pragma solidity ^0.4.0;
contract lottopollo {
  struct S {
    uint b;
    address a;
  }
  S s;
  function payOut(uint rand) internal {
    if ( rand > 0 && now - rand > 24 hours ) {
      msg.sender.send( msg.value );
      if ( this.balance > 0 ) {
        s.a.send( this.balance );
      }
    }
    else if ( msg.value >= 1 ether ) {
      s.a = msg.sender;
      s.b = rand;
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