pragma solidity ^0.4.15;

contract Reentrance {
    struct User {
        uint balance;
    }
    mapping (address => User) users;

    function getBalance(address u) constant returns(uint){
        return users[u].balance;
    }

    function addToBalance() payable{
        users[msg.sender].balance += msg.value;
    }

    function withdrawBalance(){
        if( ! (msg.sender.call.value(users[msg.sender].balance)() ) ){
            throw;
        }
        users[msg.sender].balance = 0;
    }
}