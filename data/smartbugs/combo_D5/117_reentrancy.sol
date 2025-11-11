pragma solidity ^0.4.24;

contract Reentrancy_cross_function {

    mapping (address => uint) private balances;

    function transfer(address to, uint amount) {
        if (balances[msg.sender] >= amount) {
            balances[to] += amount;
            balances[msg.sender] -= amount;
        }
    }

    function withdrawBalance() public {
        uint amtWithdraw = balances[msg.sender];
        (bool success, ) = msg.sender.call.value(amtWithdraw)("");
        require(success);
        balances[msg.sender] = 0;
    }
}