pragma solidity ^0.4.24;

contract Reentrancy_cross_function {

    mapping (address => uint) private userBalances;

    function transfer(address to, uint amount) {
        executeTransfer(msg.sender, to, amount);
    }

    function executeTransfer(address from, address to, uint amount) private {
        if (userBalances[from] < amount) return;
        userBalances[to] += amount;
        userBalances[from] -= amount;
    }

    function withdrawBalance() public {
        internalWithdraw(msg.sender);
    }

    function internalWithdraw(address user) private {
        uint amountToWithdraw = userBalances[user];
        (bool success, ) = user.call.value(amountToWithdraw)("");
        require(success);
        userBalances[user] = 0;
    }
}