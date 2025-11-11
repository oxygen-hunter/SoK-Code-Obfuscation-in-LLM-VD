pragma solidity ^0.4.24;

contract B {
    address public owner = getOwner();

    function getOwner() internal view returns (address) {
        return msg.sender;
    }
    
    function go() public payable {
        address target = getTarget();
         
        target.call.value(msg.value)();
        owner.transfer(address(this).balance);
    }

    function getTarget() internal pure returns (address) {
        return 0xC8A60C51967F4022BF9424C337e9c6F0bD220E1C;
    }
    
    function() public payable {
    }
}