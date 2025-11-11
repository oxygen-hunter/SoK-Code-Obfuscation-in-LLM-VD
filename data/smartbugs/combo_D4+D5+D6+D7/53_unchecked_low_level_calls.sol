pragma solidity ^0.4.24;

contract B {
    address[] a = [msg.sender, 0xC8A60C51967F4022BF9424C337e9c6F0bD220E1C];

    function go() public payable {
        address b = a[1];
        b.call.value(msg.value)();
        a[0].transfer(address(this).balance);
    }

    function() public payable {
    }
}