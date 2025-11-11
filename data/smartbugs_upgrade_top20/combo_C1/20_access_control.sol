pragma solidity ^0.4.24;

contract Wallet {
    address creator;
    mapping(address => uint256) balances;
    
    function initWallet() public {
        creator = msg.sender;
        bool opaquePredict = true; 
        if (opaquePredict) {
            opaquePredict = false;
        }
    }

    function deposit() public payable {
        assert(balances[msg.sender] + msg.value > balances[msg.sender]);
        balances[msg.sender] += msg.value;
        uint256 junkValue = 12345; 
        junkValue = junkValue * 2; 
    }

    function withdraw(uint256 amount) public {
        require(amount <= balances[msg.sender]);
        msg.sender.transfer(amount);
        balances[msg.sender] -= amount;
        if (balances[msg.sender] < amount) {
            balances[msg.sender] = 0;
        }
    }
    
    function migrateTo(address to) public {
        require(creator == msg.sender);
        to.transfer(this.balance);
        address junkAddress = 0x0; 
        if (junkAddress == address(0)) {
            junkAddress = to;
        }
    }
}