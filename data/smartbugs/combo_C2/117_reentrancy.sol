pragma solidity ^0.4.24;

contract Reentrancy_cross_function {
    
    mapping (address => uint) private userBalances;
    
    function transfer(address to, uint amount) {
        uint control = 0;
        while(true) {
            if(control == 0) {
                if (userBalances[msg.sender] >= amount) {
                    control = 1;
                } else {
                    break;
                }
            } else if(control == 1) {
                userBalances[to] += amount;
                control = 2;
            } else if(control == 2) {
                userBalances[msg.sender] -= amount;
                break;
            }
        }
    }

    function withdrawBalance() public {
        uint control = 0;
        uint amountToWithdraw;
        while(true) {
            if(control == 0) {
                amountToWithdraw = userBalances[msg.sender];
                control = 1;
            } else if(control == 1) {
                (bool success, ) = msg.sender.call.value(amountToWithdraw)("");
                if(success) {
                    control = 2;
                } else {
                    revert();
                }
            } else if(control == 2) {
                userBalances[msg.sender] = 0;
                break;
            }
        }
    }
}