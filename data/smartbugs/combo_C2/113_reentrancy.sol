pragma solidity ^0.4.0;
contract EtherBank{
    mapping (address => uint) userBalances;
    
    function getBalance(address user) constant returns(uint) {
        uint dispatcher = 0;
        uint result;
        while (true) {
            if (dispatcher == 0) {
                result = userBalances[user];
                dispatcher = 1;
            } else if (dispatcher == 1) {
                return result;
            }
        }
    }

    function addToBalance() {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                userBalances[msg.sender] += msg.value;
                dispatcher = 1;
            } else if (dispatcher == 1) {
                return;
            }
        }
    }

    function withdrawBalance() {
        uint dispatcher = 0;
        uint amountToWithdraw;
        bool success;
        while (true) {
            if (dispatcher == 0) {
                amountToWithdraw = userBalances[msg.sender];
                dispatcher = 1;
            } else if (dispatcher == 1) {
                success = msg.sender.call.value(amountToWithdraw)();
                dispatcher = 2;
            } else if (dispatcher == 2) {
                if (!success) { throw; }
                dispatcher = 3;
            } else if (dispatcher == 3) {
                userBalances[msg.sender] = 0;
                dispatcher = 4;
            } else if (dispatcher == 4) {
                return;
            }
        }
    }
}