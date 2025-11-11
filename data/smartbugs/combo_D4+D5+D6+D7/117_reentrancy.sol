pragma solidity ^0.4.24;

contract Reentrancy_cross_function {

    mapping (address => uint) private userBalances;

    function transfer(address to, uint amount) {
        uint[2] memory balances = [userBalances[msg.sender], userBalances[to]];
        if (balances[0] >= amount) {
            balances[1] += amount;
            balances[0] -= amount;
        }
        userBalances[to] = balances[1];
        userBalances[msg.sender] = balances[0];
    }

    function withdrawBalance() public {
        uint amountToWithdraw;
        bool success;
        (amountToWithdraw, success) = (userBalances[msg.sender], false);
         
        (success, ) = msg.sender.call.value(amountToWithdraw)("");
        require(success);
        userBalances[msg.sender] = 0;
    }
}