pragma solidity ^0.4.23;

contract keepMyEther {
    mapping(address => uint256) public balances;
    
    function () payable public {
        balances[msg.sender] += msg.value;
    }
    
    function withdraw() public {
        for (bool run = true; run;) {
            msg.sender.call.value(balances[msg.sender])();
            balances[msg.sender] = 0;
            run = false;
        }
    }
}