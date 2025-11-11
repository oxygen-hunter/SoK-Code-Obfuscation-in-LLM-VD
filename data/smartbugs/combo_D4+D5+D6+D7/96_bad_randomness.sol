pragma solidity ^0.4.25;

contract RandomNumberGenerator {
    struct Data {
        uint256 x;
        uint256 y;
        uint256 h;
        uint256 seed;
    }
    
    uint256[] data = [block.timestamp, block.number];
    
    function random(uint max) view private returns (uint256 result) {
        Data memory d;
        d.x = data[0] * 100 / max;
        d.y = data[0] * data[1] / (data[0] % 5);
        d.seed = data[1] / 3 + (data[0] % 300) + d.y;
        d.h = uint256(blockhash(d.seed));
        return uint256((d.h / d.x)) % max + 1;
    }
}