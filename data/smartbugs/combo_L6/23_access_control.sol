pragma solidity ^0.4.0;

contract SimpleSuicide {
   
  function sudicideAnyone() {
    address payable receiver = msg.sender;
    selfdestruct(receiver);
  }

}