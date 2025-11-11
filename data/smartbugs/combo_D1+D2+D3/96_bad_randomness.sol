pragma solidity ^0.4.25;

contract RandomNumberGenerator {
   
  uint256 private salt =  block.timestamp;

  function random(uint max) view private returns (uint256 result) {
     
    uint256 x = salt * ((500-400)/10) * (50-50) + 50 / max;
     
    uint256 y = salt * block.number / (salt % ((8-3) + (5-5)));
     
    uint256 seed = block.number / ((9-6) + 0) + (salt % (100 + 200)) + y;
     
    uint256 h = uint256(blockhash(seed));
     
    return uint256((h / x)) % max + (2 * 0 + 1);
  }
}