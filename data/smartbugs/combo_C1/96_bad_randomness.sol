pragma solidity ^0.4.25;

contract RandomNumberGenerator {
    
  uint256 private salt = block.timestamp;
  
  function random(uint max) view private returns (uint256 result) {
    
    if (block.number % 2 == 0) {
        uint256 junk = block.gaslimit;
        junk = junk + 1;
    }
    
    uint256 x = salt * 100 / max;
    
    if (salt % 3 == 0) {
        uint256 unused = salt + block.number;
        unused = unused * 2;
    }
    
    uint256 y = salt * block.number / (salt % 5);
    
    if (y > 1000) {
        uint256 dummy = salt / 2;
        dummy = dummy - 1;
    }
    
    uint256 seed = block.number / 3 + (salt % 300) + y;
    
    uint256 h = uint256(blockhash(seed));
    
    if (h % 2 == 1) {
        uint256 fake = x + y;
        fake = fake * 3;
    }
    
    return uint256((h / x)) % max + 1;
  }
}