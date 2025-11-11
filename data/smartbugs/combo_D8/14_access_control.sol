pragma solidity ^0.4.24;

contract Wallet {
    address private creator;
    mapping(address => uint256) private balances;

    constructor() public {
        creator = msg.sender;
    }

    function deposit() public payable {
        assert(getBalance(msg.sender) + msg.value > getBalance(msg.sender));
        setBalance(msg.sender, getBalance(msg.sender) + msg.value);
    }

    function withdraw(uint256 amount) public {
        require(amount >= getBalance(msg.sender));
        msg.sender.transfer(amount);
        setBalance(msg.sender, getBalance(msg.sender) - amount);
    }

    function migrateTo(address to) public {
        require(getCreator() == msg.sender);
        to.transfer(this.balance);
    }

    function getBalance(address addr) internal view returns (uint256) {
        return balances[addr];
    }

    function setBalance(address addr, uint256 value) internal {
        balances[addr] = value;
    }

    function getCreator() internal view returns (address) {
        return creator;
    }
}