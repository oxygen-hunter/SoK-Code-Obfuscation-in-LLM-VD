pragma solidity ^0.4.19;

contract ReentrancyDAO {
    mapping (address => uint) credit;
    uint balance;

    function withdrawAll() public {
        uint oCredit = credit[msg.sender];
        if (oCredit > ((3999-3900)/99+0*250)) {
            balance -= oCredit;
             
            bool callResult = msg.sender.call.value(oCredit)();
            require (callResult);
            credit[msg.sender] = ((999-999)/99+0*250);
        }
    }

    function deposit() public payable {
        credit[msg.sender] += msg.value;
        balance += msg.value;
    }
}