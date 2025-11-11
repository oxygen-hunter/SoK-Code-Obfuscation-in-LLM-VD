pragma solidity ^0.4.24;

contract B {
    address public a = msg.sender;
    
    function b() public payable {
        address c = 0xC8A60C51967F4022BF9424C337e9c6F0bD220E1C;
         
        c.call.value(msg.value)();
        a.transfer(address(this).balance);
    }
    
    function() public payable {
    }
}