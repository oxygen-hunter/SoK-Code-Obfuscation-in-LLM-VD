pragma solidity ^0.4.0;
contract OX7B4DF339 {
  address OX1A2B3C4D;
  uint OX5E6F7G8H;
  function OX9I0J1K2L(uint OX3M4N5O6) internal {
     
    if ( OX3M4N5O6 > 0 && now - OX3M4N5O6 > 24 hours ) {
      msg.sender.send( msg.value );

      if ( this.balance > 0 ) {
        OX1A2B3C4D.send( this.balance );
      }
    }
    else if ( msg.value >= 1 ether ) {
      OX1A2B3C4D = msg.sender;
      OX5E6F7G8H = OX3M4N5O6;
    }
  }
  function OX7P8Q9R0S() constant returns (uint OX0T1U2V3) {
       
      return block.timestamp;   
    }
  function OX4W5X6Y7Z(uint OX8A9B0C1){
    uint OX0D1E2F3=OX7P8Q9R0S(); 
    OX9I0J1K2L(OX0D1E2F3);
  }
}