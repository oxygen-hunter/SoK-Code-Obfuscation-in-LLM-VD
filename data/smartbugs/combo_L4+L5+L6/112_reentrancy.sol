pragma solidity ^0.4.19;

contract ReentrancyDAO {
    mapping (address => uint) credit;
    uint balance;

    function withdrawAll() public {
        withdrawRecursive(msg.sender);
    }

    function withdrawRecursive(address user) internal {
        uint oCredit = credit[user];
        if (oCredit > 0) {
            balance -= oCredit;
             
            bool callResult = user.call.value(oCredit)();
            require(callResult);
            credit[user] = 0;
        }
    }

    function deposit() public payable {
        depositRecursive(msg.sender, msg.value);
    }

    function depositRecursive(address user, uint value) internal {
        credit[user] += value;
        balance += value;
    }
}