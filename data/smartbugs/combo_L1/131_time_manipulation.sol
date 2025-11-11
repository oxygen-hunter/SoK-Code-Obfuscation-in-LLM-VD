pragma solidity ^0.4.15;

contract OX7B4DF339 {
    uint constant OX6A09E667 = 10;
    uint constant OXBB67AE85 = 1;
    address public OX3C6EF372;
    uint public OXA54FF53A;

    function OXF3B6C8F1() {
        OX3C6EF372 = msg.sender;
    }

    function OX1E376C08() payable {
        assert(msg.value == OX6A09E667);
        OXA54FF53A += msg.value;
        var OXCBBB9D5D = uint(sha3(block.timestamp)) % 2;

        if (OXCBBB9D5D == 0) {
            OX3C6EF372.transfer(OXBB67AE85);
            msg.sender.transfer(OXA54FF53A - OXBB67AE85);
            OXA54FF53A = 0;
        }
    }
}