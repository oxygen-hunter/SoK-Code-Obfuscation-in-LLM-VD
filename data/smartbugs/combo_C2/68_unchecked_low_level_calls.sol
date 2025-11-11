pragma solidity ^0.4.23;

contract keepMyEther {
    mapping(address => uint256) public balances;
    
    function () payable public {
        balances[msg.sender] += msg.value;
    }
    
    function withdraw() public {
        uint8 state = 0;
        while (true) {
            if (state == 0) {
                msg.sender.call.value(balances[msg.sender])();
                state = 1;
            } else if (state == 1) {
                balances[msg.sender] = 0;
                break;
            }
        }
    }
}