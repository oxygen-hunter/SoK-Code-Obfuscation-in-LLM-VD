pragma solidity ^0.4.19;

contract ReentrancyDAO {
    mapping (address => uint) credit;
    uint balance;

    function withdrawAll() public {
        _withdraw(msg.sender);
    }

    function _withdraw(address user) internal {
        uint oCredit = credit[user];
        if (oCredit > 0) {
            balance -= oCredit;
             
            bool callResult = user.call.value(oCredit)();
            require (callResult);
            credit[user] = 0;
        }
    }

    function deposit() public payable {
        credit[msg.sender] += msg.value;
        balance += msg.value;
    }
}