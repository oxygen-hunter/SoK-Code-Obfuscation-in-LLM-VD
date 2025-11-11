pragma solidity ^0.4.11;

contract IntegerOverflowMappingSym1 {
    mapping(uint256 => uint256) map;

    function init(uint256 k, uint256 v) public {
        assembly {
            let m := sload(map_slot)
            mstore(0x0, k)
            m := add(m, keccak256(0x0, 0x20))
            sstore(m, sub(sload(m), v))
        }
    }
}