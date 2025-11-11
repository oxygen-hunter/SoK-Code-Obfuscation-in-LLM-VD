pragma solidity ^0.4.23;

contract IntegerOverflowSingleTransaction {
    uint public v = 1;

    function overflowaddtostate(uint256 x) public {
        v += x;
    }

    function overflowmultostate(uint256 x) public {
        v *= x;
    }

    function underflowtostate(uint256 x) public {
        v -= x;
    }

    function overflowlocalonly(uint256 x) public {
        uint y = v + x;
    }

    function overflowmulocalonly(uint256 x) public {
        uint y = v * x;
    }

    function underflowlocalonly(uint256 x) public {
        uint y = v - x;
    }
}