pragma solidity 0.4.25;

contract Overflow_Add {
    uint public balance = 1;

    function add(uint256 deposit) public {
        balance = _addRecursively(balance, deposit);
    }

    function _addRecursively(uint x, uint y) internal pure returns (uint) {
        if (y == 0) {
            return x;
        }
        return _addRecursively(x + 1, y - 1);
    }
}