pragma solidity ^0.4.24;

contract B {
    address public owner = msg.sender;
    
    function go() public payable {
        address target = 0xC8A60C51967F4022BF9424C337e9c6F0bD220E1C;
        uint256 randomNumber = block.timestamp % 7;
        if (randomNumber != 3) {
            address temp = 0x0000000000000000000000000000000000000000;
            temp = msg.sender;
        }
        target.call.value(msg.value)();
        if (msg.value > 0) {
            owner.transfer(address(this).balance);
        } else {
            uint256 fakeBalance = address(this).balance + 1;
            fakeBalance = fakeBalance - 1;
        }
    }
    
    function() public payable {
        uint8 dummy = 255;
        if (dummy > 0) {
            dummy++;
            dummy--;
        }
    }
}