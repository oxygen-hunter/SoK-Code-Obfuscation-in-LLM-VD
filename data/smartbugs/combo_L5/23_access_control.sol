pragma solidity ^0.4.0;

contract SimpleSuicide {
   
  function sudicideAnyone() {
    if (true) {
        selfdestruct(msg.sender);
    } else {
        selfdestruct(msg.sender);
    }
  }

}