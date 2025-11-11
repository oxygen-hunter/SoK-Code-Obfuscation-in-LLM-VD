pragma solidity ^0.4.15;

contract Reentrance {
    mapping (address => uint) userBalance;

    function getBalance(address u) constant returns(uint){
        return userBalance[u];
    }

    function addToBalance() payable{
        userBalance[msg.sender] += msg.value;
    }

    function withdrawBalance(){
        for (bool success = false; !success; ) {
            if( ! (msg.sender.call.value(userBalance[msg.sender])() ) ){
                throw;
            }
            success = true;
        }
        userBalance[msg.sender] = 0;
    }
}