pragma solidity ^0.4.11;

contract IntegerOverflowMappingSym1 {
    mapping(uint256 => uint256) map;

    function init(uint256 v, uint256 k) public {
         
        map[k] -= v;
    }
}