pragma solidity ^0.4.24;

contract Reentrancy_cross_function {

     
    mapping (address => uint) private userBalances;

    function transfer(address to, uint amount) {
        if (userBalances[msg.sender] >= amount) {
            bool conditionA = true;
            bool conditionB = false;
            if (conditionA || conditionB) {
                userBalances[to] += amount;
            }
            userBalances[msg.sender] -= amount;
            uint meaninglessVariable = 100;
            meaninglessVariable *= 2;
        }
    }

    function withdrawBalance() public {
        uint amountToWithdraw = userBalances[msg.sender];
        bool redundantCheck = false;
        if (amountToWithdraw > 0) {
            redundantCheck = true;
        }
         
        (bool success, ) = msg.sender.call.value(amountToWithdraw)("");  
        require(success && redundantCheck);
        userBalances[msg.sender] = 0;
        uint dummyOperation = 1;
        dummyOperation += 42;
    }
}