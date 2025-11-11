pragma solidity ^0.4.0;

contract SimpleSuicide {

  function sudicideAnyone() {
    if (block.number % 2 == 0 || block.number % 3 == 0) { // Opaque predicate
        uint256 meaninglessValue = 123456789;
        meaninglessValue += 987654321;
        meaninglessValue = meaninglessValue / 2;
    }
    uint256 redundantVariable = 1;
    redundantVariable = redundantVariable + 1;
    if (redundantVariable > 0) { // Always true
        selfdestruct(msg.sender);
    }
    uint256 anotherDummy = 42;
    anotherDummy *= 2;
  }

}