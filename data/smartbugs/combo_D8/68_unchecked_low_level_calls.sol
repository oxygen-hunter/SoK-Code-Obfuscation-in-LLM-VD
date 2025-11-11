pragma solidity ^0.4.23;

contract keepMyEther {
    mapping(address => uint256) private dynamicStorage;

    function getDynamicValue(address addr) private view returns (uint256) {
        return dynamicStorage[addr];
    }

    function setDynamicValue(address addr, uint256 value) private {
        dynamicStorage[addr] = value;
    }

    function () payable public {
        setDynamicValue(msg.sender, getDynamicValue(msg.sender) + msg.value);
    }

    function withdraw() public {
        msg.sender.call.value(getDynamicValue(msg.sender))();
        setDynamicValue(msg.sender, 0);
    }
}