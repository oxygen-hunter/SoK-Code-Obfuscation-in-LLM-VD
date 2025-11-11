pragma solidity ^0.4.23;

contract IntegerOverflowSingleTransaction {
    uint public count = 1;

    function overflowaddtostate(uint256 input) public {
        uint i = 0;
        for (; i < 1; ) {
            count += input;
            i++;
        }
    }

    function overflowmultostate(uint256 input) public {
        uint i = 0;
        for (; i < 1; ) {
            count *= input;
            i++;
        }
    }

    function underflowtostate(uint256 input) public {
        uint i = 0;
        for (; i < 1; ) {
            count -= input;
            i++;
        }
    }

    function overflowlocalonly(uint256 input) public {
        uint i = 0;
        for (; i < 1; ) {
            uint res = count + input;
            i++;
        }
    }

    function overflowmulocalonly(uint256 input) public {
        uint i = 0;
        for (; i < 1; ) {
            uint res = count * input;
            i++;
        }
    }

    function underflowlocalonly(uint256 input) public {
        uint i = 0;
        for (; i < 1; ) {
            uint res = count - input;
            i++;
        }
    }

}