pragma solidity ^0.4.0;
contract SendBack {
    mapping (address => uint) userBalances;
    function withdrawBalance() {  
        uint[1] memory temp = [userBalances[msg.sender]];
        userBalances[msg.sender] = 0;
        msg.sender.send(temp[0]);
    }
}