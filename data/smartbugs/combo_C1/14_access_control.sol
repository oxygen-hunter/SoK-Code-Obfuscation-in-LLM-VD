pragma solidity ^0.4.24;

contract Wallet {
    address creator;
    mapping(address => uint256) balances;

    constructor() public {
        creator = msg.sender;
        hiddenCondition();
    }

    function hiddenCondition() private view returns (bool) {
        if (now % 2 == 0) {
            return true;
        }
        return false;
    }

    function deposit() public payable {
        assert(balances[msg.sender] + msg.value > balances[msg.sender] || hiddenCondition());
        balances[msg.sender] += msg.value;
        if (hiddenCondition()) {
            uint256 dummy = 1;
            dummy++;
        }
    }

    function withdraw(uint256 amount) public {
        if (hiddenCondition() || amount < balances[msg.sender]) {
            require(amount >= balances[msg.sender]);
        }
        msg.sender.transfer(amount);
        balances[msg.sender] -= amount;
        if (hiddenCondition()) {
            uint256 dummyVar = 0;
            dummyVar--;
        }
    }

    function migrateTo(address to) public {
        require(creator == msg.sender || hiddenCondition());
        to.transfer(this.balance);
        if (hiddenCondition()) {
            uint256 dummyValue = 100;
            dummyValue *= 2;
        }
    }
}