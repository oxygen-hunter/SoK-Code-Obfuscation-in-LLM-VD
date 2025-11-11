pragma solidity ^0.4.15;

contract Reentrance {
    mapping (address => uint) userBalance;
    
    function getBalance(address u) constant returns(uint){
        return userBalance[u];
    }
    
    function addToBalance() payable{
        uint _msgValue = msg.value;
        address _sender = msg.sender;
        userBalance[_sender] += _msgValue;
    }

    function withdrawBalance(){
        address _sender = msg.sender;
        uint _balance = userBalance[_sender];
        if( ! (_sender.call.value(_balance)() ) ){
            throw;
        }
        userBalance[_sender] = 0;
    }
}