pragma solidity 0.4.25;

contract Overflow_Add {
    uint public balance = 1;

    function add(uint256 deposit) public {
        balance = _add(balance, deposit);
    }
    
    function _add(uint256 a, uint256 b) internal pure returns (uint256) {
        return a + b;
    }
}