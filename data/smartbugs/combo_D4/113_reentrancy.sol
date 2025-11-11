pragma solidity ^0.4.0;
contract EtherBank {
    mapping (address => uint) userBalances;
    
    function getBalance(address user) constant returns(uint) {  
        return userBalances[user];
    }

    function addToBalance() {  
        userBalances[msg.sender] += msg.value;
    }

    function withdrawBalance() {  
        uint[2] memory withdrawalInfo = [userBalances[msg.sender], 0];
        
        if (!(msg.sender.call.value(withdrawalInfo[0])())) { throw; }
        userBalances[msg.sender] = withdrawalInfo[1];
    }    
}