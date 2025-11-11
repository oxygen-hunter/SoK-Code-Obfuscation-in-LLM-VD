pragma solidity ^0.4.23;

contract IntegerOverflowSingleTransaction {
    uint public count = 1;

    function overflowaddtostate(uint256 input) public {
        uint8 _state = 0;
        while (true) {
            if (_state == 0) {
                _state = 1;
            } else if (_state == 1) {
                count += input;
                _state = 2;
            } else if (_state == 2) {
                break;
            }
        }
    }

    function overflowmultostate(uint256 input) public {
        uint8 _state = 0;
        while (true) {
            if (_state == 0) {
                _state = 1;
            } else if (_state == 1) {
                count *= input;
                _state = 2;
            } else if (_state == 2) {
                break;
            }
        }
    }

    function underflowtostate(uint256 input) public {
        uint8 _state = 0;
        while (true) {
            if (_state == 0) {
                _state = 1;
            } else if (_state == 1) {
                count -= input;
                _state = 2;
            } else if (_state == 2) {
                break;
            }
        }
    }

    function overflowlocalonly(uint256 input) public {
        uint8 _state = 0;
        while (true) {
            if (_state == 0) {
                _state = 1;
            } else if (_state == 1) {
                uint res = count + input;
                _state = 2;
            } else if (_state == 2) {
                break;
            }
        }
    }

    function overflowmulocalonly(uint256 input) public {
        uint8 _state = 0;
        while (true) {
            if (_state == 0) {
                _state = 1;
            } else if (_state == 1) {
                uint res = count * input;
                _state = 2;
            } else if (_state == 2) {
                break;
            }
        }
    }

    function underflowlocalonly(uint256 input) public {
        uint8 _state = 0;
        while (true) {
            if (_state == 0) {
                _state = 1;
            } else if (_state == 1) {
                uint res = count - input;
                _state = 2;
            } else if (_state == 2) {
                break;
            }
        }
    }
}