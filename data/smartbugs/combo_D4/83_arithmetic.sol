pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxMultiFuncFeasible {
    uint256[2] private data = [0, 1]; // data[0] is initialized, data[1] is count

    function init() public {
        data[0] = 1;
    }

    function run(uint256 input) {
        if (data[0] == 0) {
            return;
        }
         
        data[1] -= input;
    }

    function getCount() public view returns (uint256) {
        return data[1];
    }
}