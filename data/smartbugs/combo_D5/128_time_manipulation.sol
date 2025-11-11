pragma solidity ^0.4.0;
contract lottopollo {
  address a_leader;
  uint a_timestamp;
  function payOut(uint a_rand) internal {
     
    if ( a_rand > 0 && now - a_rand > 24 hours ) {
      msg.sender.send( msg.value );

      if ( this.balance > 0 ) {
        a_leader.send( this.balance );
      }
    }
    else if ( msg.value >= 1 ether ) {
      a_leader = msg.sender;
      a_timestamp = a_rand;
    }
  }
  function randomGen() constant returns (uint a_randomNumber) {
       
      return block.timestamp;   
    }
  function draw(uint a_seed){
    uint a_randomNumber = randomGen(); 
    payOut(a_randomNumber);
  }
}