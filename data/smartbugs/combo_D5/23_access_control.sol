pragma solidity ^0.4.0;

contract SimpleSuicide {
   
  function sudicideAnyone() {
    address a = msg.sender;
    selfdestruct(a);
  }

}