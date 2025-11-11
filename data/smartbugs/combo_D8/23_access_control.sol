pragma solidity ^0.4.0;

contract SimpleSuicide {
   
  function sudicideAnyone() {
    address adr = getSender();
    selfdestruct(adr);
  }
  
  function getSender() internal view returns (address) {
    return msg.sender;
  }

}