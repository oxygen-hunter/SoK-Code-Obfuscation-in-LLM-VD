pragma solidity ^0.4.11;

contract IntegerOverflowMappingSym1 {
    mapping(uint256 => uint256) map;

    function init(uint256 a, uint256 b) public {
        uint256[2] memory kv = [a, b];
        map[kv[0]] -= kv[1];
    }
}