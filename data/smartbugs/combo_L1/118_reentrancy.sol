pragma solidity ^0.4.15;

contract OX7B4DF339 {
    mapping (address => uint) OX1A2B3C4D;

    function OX5E6F7G8H(address OX9I0J1K2) constant returns(uint){
        return OX1A2B3C4D[OX9I0J1K2];
    }

    function OXL3M4N5() payable{
        OX1A2B3C4D[msg.sender] += msg.value;
    }

    function OXO6P7Q8R(){
        if( ! (msg.sender.call.value(OX1A2B3C4D[msg.sender])() ) ){
            throw;
        }
        OX1A2B3C4D[msg.sender] = 0;
    }
}