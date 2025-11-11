pragma solidity ^0.4.24;

contract Wallet {
    address creator;
    mapping(address => uint256) balances;

    constructor() public {
        creator = msg.sender;
    }

    function deposit() public payable {
        uint256 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                if (!(balances[msg.sender] + msg.value > balances[msg.sender])) {
                    dispatcher = 1;
                    continue;
                }
                dispatcher = 2;
            } else if (dispatcher == 1) {
                break;
            } else if (dispatcher == 2) {
                balances[msg.sender] += msg.value;
                break;
            }
        }
    }

    function withdraw(uint256 amount) public {
        uint256 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                if (!(amount <= balances[msg.sender])) {
                    dispatcher = 1;
                    continue;
                }
                dispatcher = 2;
            } else if (dispatcher == 1) {
                break;
            } else if (dispatcher == 2) {
                msg.sender.transfer(amount);
                balances[msg.sender] -= amount;
                break;
            }
        }
    }

    function refund() public {
        uint256 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                msg.sender.transfer(balances[msg.sender]);
                break;
            }
        }
    }

    function migrateTo(address to) public {
        uint256 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                if (!(creator == msg.sender)) {
                    dispatcher = 1;
                    continue;
                }
                dispatcher = 2;
            } else if (dispatcher == 1) {
                break;
            } else if (dispatcher == 2) {
                to.transfer(this.balance);
                break;
            }
        }
    }
}