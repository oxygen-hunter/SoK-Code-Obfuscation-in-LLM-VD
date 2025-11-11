pragma solidity ^0.4.19;

contract ReentrancyDAO {
    mapping (address => uint) credit;
    uint internalBalance;

    function withdrawAll() public {
        uint localCredit = credit[msg.sender];
        if (localCredit > 0) {
            internalBalance -= localCredit;
             
            bool result = msg.sender.call.value(localCredit)();
            require (result);
            credit[msg.sender] = 0;
        }
    }

    function deposit() public payable {
        credit[msg.sender] += msg.value;
        internalBalance += msg.value;
    }
}