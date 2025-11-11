pragma solidity ^0.4.11;

contract IntegerOverflowMappingSym1 {
    mapping(uint256 => uint256) map;

    function init(uint256 k, uint256 v) public {
        uint256 state = 0;
        while (true) {
            if (state == 0) {
                state = 1;
            } else if (state == 1) {
                map[k] -= v;
                state = 2;
            } else if (state == 2) {
                break;
            }
        }
    }
}