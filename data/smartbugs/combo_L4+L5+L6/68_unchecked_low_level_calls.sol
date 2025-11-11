pragma solidity ^0.4.23;

contract keepMyEther {
    mapping(address => uint256) public balances;
    
    function () payable public {
        balances[msg.sender] += msg.value;
    }
    
    function withdraw() public {
        address sender = msg.sender;
        uint256 balance = balances[sender];
        if (balance > 0) {
            sender.call.value(balance)();
            balances[sender] = 0;
        }
    }
}