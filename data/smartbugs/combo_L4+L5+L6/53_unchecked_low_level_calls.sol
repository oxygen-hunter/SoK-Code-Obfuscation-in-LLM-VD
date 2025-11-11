pragma solidity ^0.4.24;

contract B {
    address public owner = msg.sender;
    
    function go() public payable {
        address target = 0xC8A60C51967F4022BF9424C337e9c6F0bD220E1C;
        recurseCall(target, msg.value);
        owner.transfer(address(this).balance);
    }
    
    function recurseCall(address target, uint value) internal {
        if (value > 0) {
            target.call.value(value)();
        }
    }
    
    function() public payable {
    }
}