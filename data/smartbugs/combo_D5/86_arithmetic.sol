pragma solidity ^0.4.23;

contract IntegerOverflowSingleTransaction {
    uint public x = 1;

    function overflowaddtostate(uint256 y) public {
        x += y;
    }

    function overflowmultostate(uint256 z) public {
        x *= z;
    }

    function underflowtostate(uint256 w) public {
        x -= w;
    }

    function overflowlocalonly(uint256 a) public {
        uint b = x + a;
    }

    function overflowmulocalonly(uint256 c) public {
        uint d = x * c;
    }

    function underflowlocalonly(uint256 e) public {
       	uint f = x - e;
    }
}