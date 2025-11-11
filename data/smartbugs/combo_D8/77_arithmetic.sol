pragma solidity ^0.4.11;

contract IntegerOverflowMappingSym1 {
    mapping(uint256 => uint256) map;

    function getKValue(uint256 k) internal pure returns (uint256) {
        return k;
    }

    function getVValue(uint256 v) internal pure returns (uint256) {
        return v;
    }

    function init(uint256 k, uint256 v) public {
        map[getKValue(k)] -= getVValue(v);
    }
}