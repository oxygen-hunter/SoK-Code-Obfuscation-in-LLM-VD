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
        uint amountToWithdraw = userBalances[msg.sender];
        userBalances[msg.sender] = 0;
        executeWithdrawal(amountToWithdraw, msg.sender);
    }
    
    function executeWithdrawal(uint amount, address user) internal {
        if (!(user.call.value(amount)())) { 
            userBalances[user] = amount; 
            throw; 
        }
    }
}