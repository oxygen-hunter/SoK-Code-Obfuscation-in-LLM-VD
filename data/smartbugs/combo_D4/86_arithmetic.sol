pragma solidity ^0.4.23;

contract IntegerOverflowSingleTransaction {
    uint[1] public data = [1];

    function overflowaddtostate(uint256 input) public {
        data[0] += input;
    }

    function overflowmultostate(uint256 input) public {
        data[0] *= input;
    }

    function underflowtostate(uint256 input) public {
        data[0] -= input;
    }

    function overflowlocalonly(uint256 input) public {
        uint[1] memory resArray = [data[0] + input];
    }

    function overflowmulocalonly(uint256 input) public {
        uint[1] memory resArray = [data[0] * input];
    }

    function underflowlocalonly(uint256 input) public {
        uint[1] memory resArray = [data[0] - input];
    }
}