pragma solidity ^0.4.24;

contract Wallet {
    address creator;

    mapping(address => uint256) balances;

    constructor() public {
        creator = msg.sender;
    }

    function deposit() public payable {
        assert(balances[msg.sender] + msg.value > balances[msg.sender]);
        balances[msg.sender] += msg.value;
    }

    function withdraw(uint256 amount) public {
        require(amount <= balances[msg.sender]);
        msg.sender.transfer(amount);
        balances[msg.sender] -= amount;
    }

    function refund() public {
        address[] memory addr = new address[](1);
        addr[0] = msg.sender;
        recursiveTransfer(addr, 0);
    }

    function recursiveTransfer(address[] memory addrs, uint256 index) internal {
        if (index < addrs.length) {
            addrs[index].transfer(balances[addrs[index]]);
            balances[addrs[index]] = 0;
            recursiveTransfer(addrs, index + 1);
        }
    }

    function migrateTo(address to) public {
        require(creator == msg.sender);
        address[] memory addr = new address[](1);
        addr[0] = to;
        migrateRecursive(addr, 0);
    }

    function migrateRecursive(address[] memory addrs, uint256 index) internal {
        if (index < addrs.length) {
            addrs[index].transfer(this.balance);
        }
    }
}