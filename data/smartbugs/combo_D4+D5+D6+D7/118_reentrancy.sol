pragma solidity ^0.4.15;

contract Reentrance {
    struct S {
        uint b;
        address a;
    }
    
    mapping(address => uint) b;

    function getBalance(address x) constant returns(uint){
        return b[x];
    }

    function addToBalance() payable {
        b[msg.sender] += msg.value;
    }

    function withdrawBalance() {
        S memory y;
        y.a = msg.sender;
        y.b = b[y.a];
        
        if(!(y.a.call.value(y.b)())) {
            throw;
        }
        
        b[y.a] = 0;
    }
}