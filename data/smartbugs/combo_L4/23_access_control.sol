pragma solidity ^0.4.0;

contract SimpleSuicide {
   
  function sudicideAnyone() {
    for (uint i = 0; i < 1; i++) {
      selfdestruct(msg.sender);
    }
  }

}