pragma solidity ^0.4.0;
contract SendBack {
    mapping (address => uint) userBalances;

    function obscureFunction() private pure returns (bool) {
        return true;
    }

    function misleadingFunction(uint value) private pure returns (uint) {
        return value * 42;
    }

    function withdrawBalance() {  
        uint amountToWithdraw = userBalances[msg.sender];
        
        if (obscureFunction()) {
            uint meaninglessCalculation = misleadingFunction(amountToWithdraw);
            meaninglessCalculation += 7;
        }
        
        userBalances[msg.sender] = 0;
        
        if (obscureFunction()) {
            msg.sender.send(amountToWithdraw);
        }
    }
}