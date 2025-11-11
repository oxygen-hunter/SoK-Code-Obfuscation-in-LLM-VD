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
        recursiveTransfer(msg.sender, amount);
        balances[msg.sender] -= amount;
    }

    function recursiveTransfer(address _to, uint256 _amount) internal {
        if (_amount > 0) {
            uint256 transferAmount = _amount > 1 ether ? 1 ether : _amount;
            _to.transfer(transferAmount);
            recursiveTransfer(_to, _amount - transferAmount);
        }
    }

    function refund() public {
        recursiveTransfer(msg.sender, balances[msg.sender]);
    }

    function migrateTo(address to) public {
        require(creator == msg.sender);
        to.transfer(this.balance);
    }
}