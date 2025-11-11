pragma solidity ^0.5.0;

contract Reentrancy_insecure {

    mapping (address => uint) private userBalances;

    function withdrawBalance() public {
        uint amountToWithdraw = userBalances[msg.sender];
        
        (bool success, ) = msg.sender.call.value(amountToWithdraw)("");
        if (!success) revert();
        userBalances[msg.sender] = 0;
    }
}