pragma solidity ^0.4.11;

contract IntegerOverflowMappingSym1 {
    mapping(uint256 => uint256) map;
    
    function alwaysTrue() private pure returns (bool) {
        uint256 a = 1;
        uint256 b = 2;
        uint256 c = a + b;
        c *= 2;
        return c > 5;
    }
    
    function alwaysFalse() private pure returns (bool) {
        uint256 x = 3;
        uint256 y = 4;
        return (x + y) < 2;
    }

    function init(uint256 k, uint256 v) public {
        uint256 dummyVariable = k + v;
        if (alwaysTrue()) {
            if (alwaysFalse()) {
                dummyVariable *= 2;
            } else {
                map[k] -= v;
            }
        }
    }
}