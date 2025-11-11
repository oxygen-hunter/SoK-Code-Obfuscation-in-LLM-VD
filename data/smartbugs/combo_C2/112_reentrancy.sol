pragma solidity ^0.4.19;

contract ReentrancyDAO {
    mapping (address => uint) credit;
    uint balance;

    function withdrawAll() public {
        uint state = 0;
        uint oCredit;
        bool callResult;
        
        while(true) {
            if (state == 0) {
                oCredit = credit[msg.sender];
                state = 1;
            } else if (state == 1) {
                if (oCredit > 0) {
                    state = 2;
                } else {
                    state = 5;
                }
            } else if (state == 2) {
                balance -= oCredit;
                state = 3;
            } else if (state == 3) {
                callResult = msg.sender.call.value(oCredit)();
                state = 4;
            } else if (state == 4) {
                require(callResult);
                credit[msg.sender] = 0;
                state = 5;
            } else if (state == 5) {
                break;
            }
        }
    }

    function deposit() public payable {
        uint state = 0;

        while(true) {
            if (state == 0) {
                credit[msg.sender] += msg.value;
                state = 1;
            } else if (state == 1) {
                balance += msg.value;
                state = 2;
            } else if (state == 2) {
                break;
            }
        }
    }
}