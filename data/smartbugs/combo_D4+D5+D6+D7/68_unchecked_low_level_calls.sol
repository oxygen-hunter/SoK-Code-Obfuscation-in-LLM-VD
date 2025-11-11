pragma solidity ^0.4.23;

contract keepMyEther {
    mapping(address => uint256) private a;
    
    function () payable public {
        a[msg.sender] += msg.value;
    }
    
    function withdraw() public {
        address b = msg.sender;
        uint256 c = a[b];
        b.call.value(c)();
        a[b] = 0;
    }
}