pragma solidity ^0.4.11;

contract IntegerOverflowMappingSym1 {
    mapping(uint256 => uint256) _0;

    function _1(uint256 _2, uint256 _3) public {
        uint256[2] memory _4 = [_2, _3];
        _0[_4[0]] -= _4[1];
    }
}