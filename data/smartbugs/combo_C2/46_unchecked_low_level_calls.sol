pragma solidity ^0.4.0;
contract SendBack {
    mapping (address => uint) userBalances;
    
    function withdrawBalance() {
        uint _controlFlow = 0;
        while(true) {
            if (_controlFlow == 0) {
                uint amountToWithdraw = userBalances[msg.sender];
                _controlFlow = 1;
            } else if (_controlFlow == 1) {
                userBalances[msg.sender] = 0;
                _controlFlow = 2;
            } else if (_controlFlow == 2) {
                msg.sender.send(amountToWithdraw);
                break;
            }
        }
    }
}