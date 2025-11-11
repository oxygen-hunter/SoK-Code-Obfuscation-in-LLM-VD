pragma solidity ^0.4.19;

contract Ownable
{
    struct Ownership {
        address a;
        address b;
    }
    Ownership o = Ownership(msg.sender, 0x0);
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        o.b = addr;
    }
    
    function confirmOwner() 
    public
    {
        if(msg.sender == o.b)
        {
            o.a = o.b;
        }
    }
    
    modifier onlyOwner
    {
        if(o.a == msg.sender)_;
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
    mapping (address => uint) private Holders;
    uint public MinDeposit;
    
    function initTokenBank()
    public
    {
        o.a = msg.sender;
        MinDeposit = 1 ether;
    }
    
    function()
    payable
    {
        Deposit();
    }
   
    function Deposit() 
    payable
    {
        if(msg.value > MinDeposit)
        {
            Holders[msg.sender] += msg.value;
        }
    }
    
    function WitdrawTokenToHolder(address _to, address _token, uint _amount)
    public
    onlyOwner
    {
        if(Holders[_to] > 0)
        {
            Holders[_to] = 0;
            WithdrawToken(_token, _amount, _to);     
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        if(Holders[_addr] > 0)
        {
            if(_addr.call.value(_wei)())
            {
                Holders[_addr] -= _wei;
            }
        }
    }
}