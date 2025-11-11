pragma solidity ^0.4.23;

contract keepMyEther {
    mapping(address => uint256) public balances;
    
    function () payable public {
        uint256 value = msg.value;
        address sender = msg.sender;
        balances[sender] += value;
    }
    
    function withdraw() public {
        address sender = msg.sender;
        sender.call.value(balances[sender])();
        balances[sender] = 0;
    }
}