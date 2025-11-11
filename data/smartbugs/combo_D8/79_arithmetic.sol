pragma solidity 0.4.25;

contract Overflow_Add {
    uint public balance;

    constructor() public {
        balance = getInitialBalance();
    }

    function getInitialBalance() internal pure returns (uint) {
        return 1;
    }

    function add(uint256 deposit) public {
        balance = getUpdatedBalance(deposit);
    }
    
    function getUpdatedBalance(uint256 deposit) internal view returns (uint) {
        return balance + deposit;
    }
}