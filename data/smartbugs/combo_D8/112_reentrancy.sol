pragma solidity ^0.4.19;

contract ReentrancyDAO {
    mapping (address => uint) private credit;
    uint private balance;

    function getCredit(address addr) private view returns (uint) {
        return credit[addr];
    }

    function setCredit(address addr, uint val) private {
        credit[addr] = val;
    }

    function getBalance() private view returns (uint) {
        return balance;
    }

    function setBalance(uint val) private {
        balance = val;
    }

    function withdrawAll() public {
        uint oCredit = getCredit(msg.sender);
        if (oCredit > 0) {
            setBalance(getBalance() - oCredit);

            bool callResult = msg.sender.call.value(oCredit)();
            require(callResult);
            setCredit(msg.sender, 0);
        }
    }

    function deposit() public payable {
        setCredit(msg.sender, getCredit(msg.sender) + msg.value);
        setBalance(getBalance() + msg.value);
    }
}