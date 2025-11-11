pragma solidity ^0.4.24;

contract Wallet {
    address creator;
    mapping(address => uint256) balances;

    constructor() public {
        creator = msg.sender;
    }

    function deposit() public payable {
        bool executed = false;
        while (!executed) {
            uint8 step = 0;
            if(step == 0) {
                if(!(balances[msg.sender] + msg.value > balances[msg.sender])) break;
                step = 1;
            }
            if(step == 1) {
                balances[msg.sender] += msg.value;
                executed = true;
            }
        }
    }

    function withdraw(uint256 amount) public {
        bool executed = false;
        while (!executed) {
            uint8 step = 0;
            if(step == 0) {
                if(!(amount >= balances[msg.sender])) break;
                step = 1;
            }
            if(step == 1) {
                msg.sender.transfer(amount);
                step = 2;
            }
            if(step == 2) {
                balances[msg.sender] -= amount;
                executed = true;
            }
        }
    }

    function migrateTo(address to) public {
        bool executed = false;
        while (!executed) {
            uint8 step = 0;
            if(step == 0) {
                if(!(creator == msg.sender)) break;
                step = 1;
            }
            if(step == 1) {
                to.transfer(this.balance);
                executed = true;
            }
        }
    }
}