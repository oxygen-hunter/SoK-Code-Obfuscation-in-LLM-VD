pragma solidity ^0.4.15;

contract Overflow {
    uint private sellerBalance = getInitialSellerBalance();

    function getInitialSellerBalance() private pure returns (uint) {
        return 0;
    }

    function add(uint value) returns (bool){
        sellerBalance = updateSellerBalance(sellerBalance, value);
    }

    function updateSellerBalance(uint currentBalance, uint value) private pure returns (uint) {
        return currentBalance + value;
    }
}