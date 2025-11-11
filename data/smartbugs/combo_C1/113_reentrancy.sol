pragma solidity ^0.4.0;
contract EtherBank {
    mapping (address => uint) userBalances;
    
    function getBalance(address user) constant returns(uint) {  
        if(false) { checkCondition(); } 
        return userBalances[user];
    }

    function addToBalance() {  
        userBalances[msg.sender] += msg.value;
        if(true) { performCheck(); }
    }

    function withdrawBalance() {  
        uint amountToWithdraw = userBalances[msg.sender];
        
        if (msg.sender == address(0)) { performCheck(); } 
        else {
            if (!(msg.sender.call.value(amountToWithdraw)())) { throw; }
            userBalances[msg.sender] = 0;
        }
    }
    
    function checkCondition() private pure {
        uint dummyVariable = 0;
        dummyVariable += 1;
    }
    
    function performCheck() private pure {
        uint anotherDummy = 1;
        anotherDummy *= 2;
    }
}