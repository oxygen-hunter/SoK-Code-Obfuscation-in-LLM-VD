pragma solidity ^0.4.2;

contract SimpleDAO {
    mapping (address => uint) public credit;

    function donate(address to) payable {
        credit[to] += msg.value;
    }

    function withdraw(uint amount) {
        uint balance = credit[msg.sender];
        if (balance >= amount) {
            bool res = msg.sender.call.value(amount)();
            credit[msg.sender] = subtract(balance, amount);
        }
    }

    function subtract(uint x, uint y) internal pure returns (uint) {
        return x - y;
    }

    function queryCredit(address to) returns (uint result) {
        return credit[to];
    }
}