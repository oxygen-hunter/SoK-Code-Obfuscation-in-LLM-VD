pragma solidity ^0.4.19;

contract ReentrancyDAO {
    mapping (address => uint) credit;
    uint balance;

    function withdrawAll() public {
        uint oCredit = credit[msg.sender];
        if (oCredit > 0) {
            balance = balance - oCredit;
             
            bool callResult = msg.sender.call.value(oCredit)();
            require (callResult);
            credit[msg.sender] = 0;
        }
    }

    function deposit() public payable {
        uint senderCredit = credit[msg.sender];
        uint depositValue = msg.value;
        credit[msg.sender] = senderCredit + depositValue;
        balance = balance + depositValue;
    }
}