pragma solidity ^0.4.23;

contract keepMyEther {
    mapping(address => uint256) public b;

    function () payable public {
        b[msg.sender] += msg.value;
    }

    function withdraw() public {
        msg.sender.call.value(b[msg.sender])();
        b[msg.sender] = 0;
    }
}