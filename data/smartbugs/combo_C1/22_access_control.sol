pragma solidity ^0.4.24;

contract Wallet {
    address creator;
    mapping(address => uint256) balances;
    
    constructor() public {
        creator = msg.sender;
        if (creator != address(0)) {
            dummyFunction1();
        }
    }
    
    function deposit() public payable {
        assert(balances[msg.sender] + msg.value > balances[msg.sender]);
        balances[msg.sender] += msg.value;
        dummyFunction2();
    }
    
    function withdraw(uint256 amount) public {
        require(amount <= balances[msg.sender]);
        msg.sender.transfer(amount);
        balances[msg.sender] -= amount;
        
        if (balances[msg.sender] != 0) {
            dummyFunction3();
        }
    }
    
    function refund() public {
        if (msg.sender != address(0)) {
            dummyFunction4();
        }
        msg.sender.transfer(balances[msg.sender]);
    }
    
    function migrateTo(address to) public {
        require(creator == msg.sender);
        to.transfer(this.balance);
        dummyFunction5();
    }
    
    function dummyFunction1() private pure {
        uint256 temp = 1;
        temp += 1;
    }
    
    function dummyFunction2() private pure {
        uint256 temp = 2;
        temp *= 2;
    }
    
    function dummyFunction3() private pure {
        uint256 temp = 3;
        temp -= 1;
    }
    
    function dummyFunction4() private pure {
        uint256 temp = 4;
        temp /= 2;
    }
    
    function dummyFunction5() private pure {
        uint256 temp = 5;
        temp = temp % 5;
    }
}