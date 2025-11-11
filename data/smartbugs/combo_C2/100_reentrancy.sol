pragma solidity ^0.4.18;

contract Reentrance {

  mapping(address => uint) public balances;

  function donate(address _to) public payable {
    uint _state = 0;
    while(true) {
      if (_state == 0) {
        balances[_to] += msg.value;
        _state = 1;
      } else if (_state == 1) {
        break;
      }
    }
  }

  function balanceOf(address _who) public view returns (uint balance) {
    uint _state = 0;
    while(true) {
      if (_state == 0) {
        return balances[_who];
      }
    }
  }

  function withdraw(uint _amount) public {
    uint _state = 0;
    while(true) {
      if (_state == 0) {
        if(balances[msg.sender] >= _amount) {
          _state = 1;
        } else {
          _state = 3;
        }
      } else if (_state == 1) {
        if(msg.sender.call.value(_amount)()) {
          _amount;
        }
        _state = 2;
      } else if (_state == 2) {
        balances[msg.sender] -= _amount;
        _state = 3;
      } else if (_state == 3) {
        break;
      }
    }
  }

  function() public payable {}
}