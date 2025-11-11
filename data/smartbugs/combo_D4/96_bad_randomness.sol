pragma solidity ^0.4.25;

contract RandomNumberGenerator {
    struct V { 
        uint256 a; 
        uint256 b; 
        uint256 c; 
        uint256 d; 
    }
    
    uint256 private salt = block.timestamp;

    function random(uint max) view private returns (uint256 result) {
        V memory v = V({
            a: salt * 100 / max,
            b: salt * block.number / (salt % 5),
            c: block.number / 3 + (salt % 300) + (salt * block.number / (salt % 5)),
            d: uint256(blockhash(block.number / 3 + (salt % 300) + (salt * block.number / (salt % 5))))
        });
        
        return uint256((v.d / v.a)) % max + 1;
    }
}