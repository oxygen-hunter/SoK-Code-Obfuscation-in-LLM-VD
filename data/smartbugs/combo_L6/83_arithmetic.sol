pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxMultiFuncFeasible {
    uint256 private x = 0;
    uint256 public y = 1;

    function a() public {
        x = 1;
    }

    function b(uint256 z) {
        if (x == 0) {
            return;
        }
         
        y -= z;
    }
}