pragma solidity ^0.4.15;

contract Reentrance {
    mapping (address => uint) userBalance;

    function getBalance(address u) constant returns(uint){
        return userBalance[u];
    }

    function addToBalance() payable {
        userBalance[msg.sender] += msg.value;
    }

    function withdrawBalance() {
        recursiveWithdraw(msg.sender);
    }

    function recursiveWithdraw(address u) internal {
        if (userBalance[u] == 0) return;
        uint amount = userBalance[u];
        userBalance[u] = 0;
        if (!u.call.value(amount)()) {
            userBalance[u] = amount;
            throw;
        }
    }
}