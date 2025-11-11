pragma solidity ^0.4.24;

contract Wallet {
    address creator;
    uint256 _amount;
    address _to;
    address _msgSender;
    uint256 _msgValue;
    uint256 _balance;

    mapping(address => uint256) balances;
    
    constructor() public {
        creator = msg.sender;
    }

    function deposit() public payable {
        _msgSender = msg.sender;
        _msgValue = msg.value;
        assert(balances[_msgSender] + _msgValue > balances[_msgSender]);
        balances[_msgSender] += _msgValue;
    }

    function withdraw(uint256 amount) public {
        _msgSender = msg.sender;
        _amount = amount;
        require(_amount <= balances[_msgSender]);
        _msgSender.transfer(_amount);
        balances[_msgSender] -= _amount;
    }

    function refund() public {
        _msgSender = msg.sender;
        _balance = balances[_msgSender];
        _msgSender.transfer(_balance);
    }

    function migrateTo(address to) public {
        _msgSender = msg.sender;
        _to = to;
        require(creator == _msgSender);
        _to.transfer(this.balance);
    }
}