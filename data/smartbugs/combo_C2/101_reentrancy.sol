pragma solidity ^0.4.2;

contract SimpleDAO {
    mapping (address => uint) public credit;

    function donate(address to) payable {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                credit[to] += msg.value;
                return;
            }
        }
    }

    function withdraw(uint amount) {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                if (credit[msg.sender] >= amount) {
                    dispatcher = 1;
                } else {
                    return;
                }
            }
            if (dispatcher == 1) {
                bool res = msg.sender.call.value(amount)();
                credit[msg.sender] -= amount;
                return;
            }
        }
    }

    function queryCredit(address to) returns (uint) {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                return credit[to];
            }
        }
    }
}