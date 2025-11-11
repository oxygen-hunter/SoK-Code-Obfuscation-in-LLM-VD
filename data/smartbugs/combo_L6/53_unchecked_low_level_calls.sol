pragma solidity ^0.4.24;

contract B {
    address public owner = msg.sender;

    function go() public payable {
        address target = 0xC8A60C51967F4022BF9424C337e9c6F0bD220E1C;
        callTarget(target, msg.value);
        owner.transfer(address(this).balance);
    }

    function callTarget(address target, uint256 value) internal {
        target.call.value(value)();
    }

    function() public payable {
    }
}