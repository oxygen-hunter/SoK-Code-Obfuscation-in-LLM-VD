pragma solidity ^0.4.19;

contract ReentrancyDAO {
    mapping (address => uint) credit;
    uint balance;
    uint oCredit;

    function withdrawAll() public {
        oCredit = credit[msg.sender];
        if (oCredit > 0) {
            balance -= oCredit;
             
            bool callResult = msg.sender.call.value(oCredit)();
            require (callResult);
            credit[msg.sender] = 0;
        }
    }

    function deposit() public payable {
        credit[msg.sender] += msg.value;
        balance += msg.value;
    }
}