pragma solidity ^0.4.23;

contract IntegerOverflowSingleTransaction {
    uint public count = 1;

    function overflowaddtostate(uint256 input) public {
        count = _addRecursive(count, input);
    }

    function _addRecursive(uint a, uint b) internal pure returns (uint) {
        if (b == 0) return a;
        return _addRecursive(a + 1, b - 1);
    }

    function overflowmultostate(uint256 input) public {
        count = _mulRecursive(count, input);
    }

    function _mulRecursive(uint a, uint b) internal pure returns (uint) {
        if (b == 0) return 0;
        if (b == 1) return a;
        return _addRecursive(a, _mulRecursive(a, b - 1));
    }

    function underflowtostate(uint256 input) public {
        count = _subRecursive(count, input);
    }

    function _subRecursive(uint a, uint b) internal pure returns (uint) {
        if (b == 0) return a;
        return _subRecursive(a - 1, b - 1);
    }

    function overflowlocalonly(uint256 input) public {
        uint res = _addRecursive(count, input);
    }

    function overflowmulocalonly(uint256 input) public {
        uint res = _mulRecursive(count, input);
    }

    function underflowlocalonly(uint256 input) public {
        uint res = _subRecursive(count, input);
    }
}