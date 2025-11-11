pragma solidity ^0.4.15;

contract Reentrance {
    mapping (address => uint) private balances;

    function getBalance(address u) constant returns(uint) {
        return getUserBalance(u);
    }

    function addToBalance() payable {
        setUserBalance(msg.sender, getUserBalance(msg.sender) + msg.value);
    }

    function withdrawBalance() {
        uint amountToWithdraw = getUserBalance(msg.sender);
        if(!(msg.sender.call.value(amountToWithdraw)())) {
            throw;
        }
        setUserBalance(msg.sender, 0);
    }

    function getUserBalance(address u) private view returns (uint) {
        return balances[u];
    }

    function setUserBalance(address u, uint value) private {
        balances[u] = value;
    }
}