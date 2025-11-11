pragma solidity ^0.4.25;

contract RandomNumberGenerator {
    uint256 private salt = block.timestamp;

    function random(uint max) view private returns (uint256 result) {
        assembly {
            let x := div(mul(sload(salt_slot), 100), max)
            let y := div(mul(sload(salt_slot), number()), mod(sload(salt_slot), 5))
            let seed := add(add(div(number(), 3), mod(sload(salt_slot), 300)), y)
            let h := sload(add(0x40, seed))
            result := add(mod(div(h, x), max), 1)
        }
    }
}