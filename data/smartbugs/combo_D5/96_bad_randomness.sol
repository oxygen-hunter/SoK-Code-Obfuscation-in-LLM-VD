pragma solidity ^0.4.25;

contract RandomNumberGenerator {
   
  uint256 private salt =  block.timestamp;

  function random(uint max) view private returns (uint256 result) {
     
    uint256 x1 = salt * 100;
    uint256 x2 = max;
    uint256 x = x1 / x2;
     
    uint256 y1 = salt * block.number;
    uint256 y2 = (salt % 5);
    uint256 y = y1 / y2;
     
    uint256 seed1 = block.number / 3;
    uint256 seed2 = (salt % 300);
    uint256 seed3 = y;
    uint256 seed = seed1 + seed2 + seed3;
     
    uint256 h = uint256(blockhash(seed));
     
    uint256 result1 = (h / x);
    uint256 result2 = result1 % max;
    uint256 result3 = 1;
    return result2 + result3;
  }
}