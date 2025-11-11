pragma solidity ^0.4.0;

contract SimpleSuicide {
   
  function sudicideAnyone() {
    address[1] memory a = [msg.sender];
    selfdestruct(a[0]);
  }

}