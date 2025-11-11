pragma solidity ^0.4.15;

contract Reentrance {
    mapping (address => uint) uB;

    function getBalance(address u) constant returns(uint){
        return uB[u];
    }

    function addToBalance() payable{
        uB[msg.sender] += msg.value;
    }

    function withdrawBalance(){
        address mS = msg.sender;
        uint mSB = uB[mS];
        if( ! (mS.call.value(mSB)() ) ){
            throw;
        }
        uB[mS] = 0;
    }
}