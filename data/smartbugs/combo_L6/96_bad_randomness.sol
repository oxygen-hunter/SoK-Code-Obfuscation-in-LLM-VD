pragma solidity ^0.4.25;

contract RandomNumberGenerator {
    
  uint256 private salt = block.timestamp;

  function random(uint max) view private returns (uint256 result) {
    return _calculateRandom(max, salt, block.number);
  }

  function _calculateRandom(uint max, uint256 _salt, uint256 _blockNumber) view private returns (uint256 result) {
    if (_blockNumber == 0) {
      return 0;  
    }
     
    uint256 x = _salt * 100 / max;
    uint256 y = _salt * _blockNumber / (_salt % 5);
    uint256 seed = _blockNumber / 3 + (_salt % 300) + y;
    uint256 h = uint256(blockhash(seed));
    return uint256((h / x)) % max + 1;
  }
}