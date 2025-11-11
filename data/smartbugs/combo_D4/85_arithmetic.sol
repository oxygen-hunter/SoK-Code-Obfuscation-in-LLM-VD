pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxOneFuncFeasible {
    uint256[2] private data = [0, 1];

    function run(uint256 input) public {
        if (data[0] == 0) {
            data[0] = 1;
            return;
        }
         
        data[1] -= input;
    }

    function count() public view returns (uint256) {
        return data[1];
    }
}