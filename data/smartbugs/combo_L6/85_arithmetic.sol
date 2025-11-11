pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxOneFuncFeasible {
    uint256 private i = 0;
    uint256 public c = 1;

    function r(uint256 n) public {
        if (i == 0) {
            i = 1;
        } else {
            c -= n;
        }
    }
}