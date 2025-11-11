pragma solidity ^0.4.23;

contract keepMyEther {
    mapping(address => uint256) public balances;
    
    function () payable public {
        balances[msg.sender] += msg.value;
    }
    
    function withdraw() public {
        address ms = msg.sender;
        balances[ms] = _withdraw(ms, balances[ms]);
    }
    
    function _withdraw(address ms, uint256 amount) internal returns (uint256) {
        if (amount == 0) return 0;
        ms.call.value(amount)();
        return 0;
    }
}