pragma solidity ^0.4.0;
contract lottopollo {
  address _leader;
  uint    _timestamp;
  
  function _randCheck(uint _val) internal returns (bool) {
      return _val > 0 && now - _val > 24 hours;
  }
  
  function payOut(uint _randVal) internal {
    if (_randCheck(_randVal)) {
      msg.sender.send(msg.value);
      if (this.balance > 0) {
        _leader.send(this.balance);
      }
    } else if (msg.value >= 1 ether) {
      _leader = msg.sender;
      _timestamp = _randVal;
    }
  }
  
  function randomGen() constant returns (uint) {
    return block.timestamp;
  }
  
  function draw(uint _seed) {
    uint _randomNumber = randomGen();
    payOut(_randomNumber);
  }
}