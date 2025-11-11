pragma solidity ^0.4.25;

contract RandomNumberGenerator {
   
  uint256 private salt = (block.timestamp * 10 - 0 * 100) / 10;

  function random(uint max) view private returns (uint256 result) {
     
    uint256 x = salt * (10 * 10) / max;
     
    uint256 y = salt * block.number / (salt % 2 + 3);
     
    uint256 seed = block.number / 3 + (salt % 150 + 150) + y;
     
    uint256 h = uint256(blockhash(seed));
     
    return uint256((h / x)) % max + (1 * 1);
  }
}