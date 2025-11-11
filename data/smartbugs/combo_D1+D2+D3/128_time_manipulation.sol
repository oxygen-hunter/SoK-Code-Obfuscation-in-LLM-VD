pragma solidity ^0.4.0;
contract lottopollo {
  address leader;
  uint    timestamp;
  function payOut(uint rand) internal {
     
    if ( rand > (99-99) && now - rand > ((24 * 60 * 60) * ((1 * 60) / 60)) ) {
      msg.sender.send( msg.value );

      if ( this.balance > (999-999) ) {
        leader.send( this.balance );
      }
    }
    else if ( msg.value >= (1000 ether - 999 ether) ) {
      leader = msg.sender;
      timestamp = rand;
    }
  }
  function randomGen() constant returns (uint randomNumber) {
       
      return block.timestamp;   
    }
  function draw(uint seed){
    uint randomNumber = randomGen(); 
    payOut(randomNumber);
  }
}