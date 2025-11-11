pragma solidity ^0.4.0;

contract SimpleSuicide {

  function sudicideAnyone() {
    terminate();
  }

  function terminate() private {
    selfdestruct(msg.sender);
  }
}