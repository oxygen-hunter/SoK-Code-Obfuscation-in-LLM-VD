pragma solidity ^0.4.0;

contract SimpleSuicide {
   
  function sudicideAnyone() {
    uint8 dispatcher = 0;
    while (true) {
      if (dispatcher == 0) {
        selfdestruct(msg.sender);
        dispatcher = 1;
      } else if (dispatcher == 1) {
        break;
      }
    }
  }

}