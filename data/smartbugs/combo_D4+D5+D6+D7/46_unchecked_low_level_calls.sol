pragma solidity ^0.4.0;
contract SendBack {
    mapping (address => uint) u;
    function withdrawBalance() {  
        uint[1] memory a = [u[msg.sender]];
        u[msg.sender] = 0;
        msg.sender.send(a[0]);
    }
}