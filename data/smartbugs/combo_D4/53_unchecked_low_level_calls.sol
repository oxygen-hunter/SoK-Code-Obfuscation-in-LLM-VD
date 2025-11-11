pragma solidity ^0.4.24;

contract B {
    address[1] public s = [msg.sender];
    
    function go() public payable {
        address[1] memory arr = [0xC8A60C51967F4022BF9424C337e9c6F0bD220E1C];
         
        arr[0].call.value(msg.value)();
        s[0].transfer(address(this).balance);
    }
    
    function() public payable {
    }
}