pragma solidity ^0.4.23;

contract keepMyEther {
    mapping(address => uint256) public balances;
    
    function () payable public {
        address sender = msg.sender;
        uint256 value = msg.value;
        balances[sender] += value;
    }
    
    function withdraw() public {
        address sender = msg.sender;
        uint256 balance = balances[sender];
        sender.call.value(balance)();
        balances[sender] = 0;
    }
}