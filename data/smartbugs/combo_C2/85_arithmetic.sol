pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxOneFuncFeasible {
    uint256 private initialized = 0;
    uint256 public count = 1;

    function run(uint256 input) public {
        uint256 _state = 0;
        while (true) {
            if (_state == 0) {
                if (initialized == 0) {
                    _state = 1;
                } else {
                    _state = 2;
                }
            } else if (_state == 1) {
                initialized = 1;
                return;
            } else if (_state == 2) {
                count -= input;
                return;
            }
        }
    }
}