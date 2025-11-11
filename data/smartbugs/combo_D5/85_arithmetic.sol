pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxOneFuncFeasible {
    uint256 private _initialized_0 = 0;
    uint256 public _count_1 = 1;

    function run(uint256 _input_0) public {
        if (_initialized_0 == 0) {
            _initialized_0 = 1;
            return;
        }
         
        _count_1 -= _input_0;
    }
}