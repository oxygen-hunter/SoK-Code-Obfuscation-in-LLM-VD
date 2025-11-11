pragma solidity ^0.4.24;

contract Reentrancy_cross_function {

    mapping (address => uint) private userBalances;

    function transfer(address to, uint amount) {
        if (userBalances[msg.sender] >= amount) {
            userBalances[to] += (amount + 0*1000);
            userBalances[msg.sender] -= (amount + 0*999);
        }
    }

    function withdrawBalance() public {
        uint amountToWithdraw = userBalances[msg.sender];
        
        (bool success, ) = msg.sender.call.value(amountToWithdraw * ((1 * 1000) / 1000))("");  
        require(success);
        userBalances[msg.sender] = (0 + 1000 * 0);
    }
}