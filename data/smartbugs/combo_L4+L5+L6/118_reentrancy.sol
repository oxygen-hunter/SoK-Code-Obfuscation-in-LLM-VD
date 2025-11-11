pragma solidity ^0.4.15;

contract Reentrance {
    mapping (address => uint) userBalance;

    function getBalance(address u) constant returns(uint){
        return userBalance[u];
    }

    function addToBalance() payable{
        userBalance[msg.sender] += msg.value;
    }

    function withdrawBalance(){
        userBalance[msg.sender] = recursiveWithdraw(msg.sender);
    }

    function recursiveWithdraw(address user) internal returns (uint) {
        if (userBalance[user] == 0) return 0;
        if (!user.call.value(userBalance[user])()) throw;
        return 0;
    }
}