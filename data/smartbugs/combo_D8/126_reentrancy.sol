pragma solidity ^0.5.0;

contract Reentrancy_insecure {

    mapping (address => uint) private userBalances;

    function getUserBalance() private view returns (uint) {
        return userBalances[msg.sender];
    }

    function resetUserBalance() private {
        userBalances[msg.sender] = 0;
    }

    function withdrawBalance() public {
        uint amountToWithdraw = getUserBalance();
        (bool success, ) = msg.sender.call.value(amountToWithdraw)("");
        require(success);
        resetUserBalance();
    }
}