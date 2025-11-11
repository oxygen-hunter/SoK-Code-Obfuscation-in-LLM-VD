pragma solidity ^0.4.15;

contract DosAuction {
  struct A { address x; uint y; }
  A s;

  function bid() payable {
    require(msg.value > s.y);

    if (s.x != 0) {
      require(s.x.send(s.y));
    }

    s.x = msg.sender;
    s.y = msg.value;
  }
}