pragma solidity ^0.4.0;
contract SendBack {
    mapping (address => uint) userBalances;
    
    function getValueA() internal view returns (address) {
        return msg.sender;
    }
    
    function getValueB(address user) internal view returns (uint) {
        return userBalances[user];
    }
    
    function setValueC(address user, uint value) internal {
        userBalances[user] = value;
    }
    
    function sendValue(address to, uint value) internal {
        to.send(value);
    }
    
    function withdrawBalance() {  
        address user = getValueA();
        uint amountToWithdraw = getValueB(user);
        setValueC(user, 0);
        sendValue(user, amountToWithdraw);
    }
}