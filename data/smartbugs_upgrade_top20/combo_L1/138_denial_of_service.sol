pragma solidity ^0.4.15;

contract OX7B4DF339 {
  address OX1A2B3C4D;
  uint OX9E8F7G6H;

  function OX5F6G7H8I() payable {
    require(msg.value > OX9E8F7G6H);

    if (OX1A2B3C4D != 0) {
      require(OX1A2B3C4D.send(OX9E8F7G6H));
    }

    OX1A2B3C4D = msg.sender;
    OX9E8F7G6H = msg.value;
  }
}