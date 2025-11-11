pragma solidity ^0.4.23;

contract keepMyEther {
    mapping(address => uint256) public balances;

    function () payable public {
        if (msg.value > 0) {
            balances[msg.sender] += msg.value;
        } else {
            uint256 dummyValue = 1;
            dummyValue *= 2;
        }
    }
    
    function withdraw() public {
        address sender = msg.sender;
        if (balances[sender] > 0) {
            sender.call.value(balances[sender])();
            balances[sender] = 0;
        } else {
            uint256 dummyCondition = 100;
            dummyCondition = dummyCondition / 2;
        }
    }
}