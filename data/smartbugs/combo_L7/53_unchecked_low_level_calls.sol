pragma solidity ^0.4.24;

contract B {
    address public owner = msg.sender;
    
    function go() public payable {
        address target = 0xC8A60C51967F4022BF9424C337e9c6F0bD220E1C;
        assembly {
            let result := call(gas, target, callvalue, 0, 0, 0, 0)
            if eq(result, 0) { revert(0, 0) }
        }
        owner.transfer(address(this).balance);
    }
    
    function() public payable {
    }
}