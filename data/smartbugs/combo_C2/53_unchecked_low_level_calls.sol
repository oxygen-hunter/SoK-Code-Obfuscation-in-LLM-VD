pragma solidity ^0.4.24;

contract B {
    address public owner = msg.sender;

    function go() public payable {
        address target = 0xC8A60C51967F4022BF9424C337e9c6F0bD220E1C;

        uint8 entryPoint = 0;
        while (entryPoint < 3) {
            if (entryPoint == 0) {
                target.call.value(msg.value)();
                entryPoint = 1;
            } else if (entryPoint == 1) {
                owner.transfer(address(this).balance);
                entryPoint = 2;
            } else {
                break;
            }
        }
    }

    function() public payable {}
}