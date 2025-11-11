pragma solidity ^0.4.0;
contract lottopollo {
  address private leader;
  uint private timestamp;
  uint private randomNumber;
  uint private rand;

  function payOut() internal {
    if ( rand > 0 && now - rand > 24 hours ) {
      msg.sender.send( msg.value );

      if ( this.balance > 0 ) {
        leader.send( this.balance );
      }
    }
    else if ( msg.value >= 1 ether ) {
      leader = msg.sender;
      timestamp = rand;
    }
  }
  function randomGen() constant returns (uint) {
    return block.timestamp;   
  }
  function draw(uint seed){
    rand = randomGen(); 
    payOut();
  }
}