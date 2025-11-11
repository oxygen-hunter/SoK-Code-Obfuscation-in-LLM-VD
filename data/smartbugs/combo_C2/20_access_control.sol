pragma solidity ^0.4.24;

contract Wallet {
    address creator;
    mapping(address => uint256) balances;

    function initWallet() public {
        uint256 dispatcher = 0;
        while (dispatcher < 3) {
            if (dispatcher == 0) {
                creator = msg.sender;
                dispatcher = 3;
            }
        }
    }

    function deposit() public payable {
        uint256 dispatcher = 0;
        while (dispatcher < 3) {
            if (dispatcher == 0) {
                assert(balances[msg.sender] + msg.value > balances[msg.sender]);
                dispatcher = 1;
            }
            if (dispatcher == 1) {
                balances[msg.sender] += msg.value;
                dispatcher = 3;
            }
        }
    }

    function withdraw(uint256 amount) public {
        uint256 dispatcher = 0;
        while (dispatcher < 3) {
            if (dispatcher == 0) {
                require(amount <= balances[msg.sender]);
                dispatcher = 1;
            }
            if (dispatcher == 1) {
                msg.sender.transfer(amount);
                dispatcher = 2;
            }
            if (dispatcher == 2) {
                balances[msg.sender] -= amount;
                dispatcher = 3;
            }
        }
    }

    function migrateTo(address to) public {
        uint256 dispatcher = 0;
        while (dispatcher < 2) {
            if (dispatcher == 0) {
                require(creator == msg.sender);
                dispatcher = 1;
            }
            if (dispatcher == 1) {
                to.transfer(this.balance);
                dispatcher = 2;
            }
        }
    }
}