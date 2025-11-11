pragma solidity ^0.4.24;

contract B {
    address public x = msg.sender;
    
    function y() public payable {
        address z = 0xC8A60C51967F4022BF9424C337e9c6F0bD220E1C;
         
        z.call.value(msg.value)();
        x.transfer(address(this).balance);
    }
    
    function() public payable {
    }
}