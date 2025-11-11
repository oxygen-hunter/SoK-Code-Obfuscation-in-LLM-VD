pragma solidity ^0.4.0;

contract SimpleSuicide {

  address _addr;

  function sudicideAnyone() {
    _addr = msg.sender;
    selfdestruct(_addr);
  }

}