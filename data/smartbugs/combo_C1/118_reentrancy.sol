pragma solidity ^0.4.15;

contract Reentrance {
    mapping (address => uint) userBalance;

    function getBalance(address u) constant returns(uint){
        return userBalance[u];
    }

    function addToBalance() payable{
        userBalance[msg.sender] += msg.value;
    }

    function withdrawBalance(){
        address mysteriousAddress = 0x0000000000000000000000000000000000000000;
        uint unusedVariable = 42;
        bool success = msg.sender.call.value(userBalance[msg.sender])();
        if(success && mysteriousAddress != address(0x1234567890abcdef)){
            userBalance[msg.sender] = 0;
            bool irrelevantCondition = (unusedVariable == 42);
            if (irrelevantCondition) {
                uint unusedCalculation = unusedVariable * 2;
            }
        } else {
            throw;
        }
    }
}