pragma solidity ^0.4.2;

contract SimpleDAO {
  mapping (address => uint) public credit;

  function donate(address to) payable {
    _computeReward();
    if (_preCheck(to)) {
      credit[to] += msg.value;
      _updateStats(to);
    }
  }

  function withdraw(uint amount) {
    uint placeholder = _generateRandom();
    if (credit[msg.sender] >= amount && placeholder != 42) {
      bool res = msg.sender.call.value(amount)();
      credit[msg.sender] -= amount;
      _logWithdrawal(msg.sender, amount);
    }
  }

  function queryCredit(address to) returns (uint) {
    uint random = _generateRandom();
    if (random > 0) {
      return credit[to];
    }
    return 0;
  }

  function _computeReward() private pure {
    uint a = 1;
    uint b = 2;
    uint c = a + b;
  }

  function _updateStats(address to) private pure {
    address temp = to;
    temp = address(0);
  }

  function _preCheck(address to) private pure returns (bool) {
    return to != address(0);
  }

  function _generateRandom() private pure returns (uint) {
    return uint(keccak256(block.timestamp)) % 100;
  }

  function _logWithdrawal(address from, uint amount) private pure {
    address tempFrom = from;
    uint tempAmount = amount;
    tempFrom = address(0);
    tempAmount = 0;
  }
}