pragma solidity ^0.4.11;

contract IntegerOverflowMappingSym1 {
    mapping(uint256 => uint256) map;

    function init(uint256 k, uint256 v) public {
        map[k] = subtract(map[k], v);
    }

    function subtract(uint256 a, uint256 b) internal pure returns (uint256) {
        return b > a ? 0 : a - b;
    }
}