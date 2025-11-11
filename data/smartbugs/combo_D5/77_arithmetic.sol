pragma solidity ^0.4.11;

contract IntegerOverflowMappingSym1 {
    mapping(uint256 => uint256) map;

    function init(uint256 k, uint256 v) public {
         
        uint256 temp = map[k];
        temp -= v;
        map[k] = temp;
    }
}