pragma solidity ^0.5.0;

contract Reentrancy_insecure {

    mapping (address => uint) private userBalances;

    function withdrawBalance() public {
        uint amountToWithdraw = userBalances[msg.sender];
        callWithdrawal(msg.sender, amountToWithdraw);
    }
    
    function callWithdrawal(address user, uint amount) private {
        (bool success, ) = user.call.value(amount)("");
        require(success);
        userBalances[user] = 0;
    }
}