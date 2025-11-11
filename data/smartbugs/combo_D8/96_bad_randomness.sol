pragma solidity ^0.4.25;

contract RandomNumberGenerator {
    
    function getSalt() private view returns (uint256) {
        return block.timestamp;
    }

    function random(uint max) view private returns (uint256 result) {
        
        uint256 x = getSalt() * 100 / max;
        
        uint256 y = getSalt() * block.number / (getSalt() % 5);
        
        uint256 seed = block.number / 3 + (getSalt() % 300) + y;
        
        uint256 h = uint256(blockhash(seed));
        
        return uint256((h / x)) % max + 1;
    }
}