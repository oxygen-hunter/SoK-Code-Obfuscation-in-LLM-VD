pragma solidity ^0.4.25;

contract RandomNumberGenerator {
    uint256 private salt = block.timestamp;

    function random(uint max) view private returns (uint256 result) {
        uint dispatch = 0;
        uint256 x;
        uint256 y;
        uint256 seed;
        uint256 h;
        
        while (true) {
            if (dispatch == 0) {
                x = salt * 100 / max;
                dispatch = 1;
            } else if (dispatch == 1) {
                y = salt * block.number / (salt % 5);
                dispatch = 2;
            } else if (dispatch == 2) {
                seed = block.number / 3 + (salt % 300) + y;
                dispatch = 3;
            } else if (dispatch == 3) {
                h = uint256(blockhash(seed));
                dispatch = 4;
            } else if (dispatch == 4) {
                return uint256((h / x)) % max + 1;
            }
        }
    }
}