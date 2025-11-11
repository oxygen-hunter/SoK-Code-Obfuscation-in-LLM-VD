pragma solidity ^0.4.19;

contract Ownable
{
    struct Addresses {
        address newOwner;
        address owner;
    }
    Addresses addrs = Addresses(msg.sender, msg.sender);
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        addrs.newOwner = addr;
    }
    
    function confirmOwner() 
    public
    {
        if(msg.sender==addrs.newOwner)
        {
            addrs.owner=addrs.newOwner;
        }
    }
    
    modifier onlyOwner
    {
        if(addrs.owner == msg.sender)_;
    }
}

contract Token is Ownable
{
    function WithdrawToken(address token, uint256 amount,address to)
    public 
    onlyOwner
    {
        token.call(bytes4(sha3("transfer(address,uint256)")),to,amount); 
    }
}

contract TokenBank is Token
{
    struct DepositInfo {
        uint MinDeposit;
        mapping (address => uint) Holders;
    }
    DepositInfo depositInfo;
    
    function initTokenBank()
    public
    {
        addrs.owner = msg.sender;
        depositInfo.MinDeposit = 1 ether;
    }
    
    function()
    payable
    {
        Deposit();
    }
   
    function Deposit() 
    payable
    {
        if(msg.value > depositInfo.MinDeposit)
        {
            depositInfo.Holders[msg.sender] += msg.value;
        }
    }
    
    function WitdrawTokenToHolder(address _to,address _token,uint _amount)
    public
    onlyOwner
    {
        if(depositInfo.Holders[_to] > 0)
        {
            depositInfo.Holders[_to] = 0;
            WithdrawToken(_token,_amount,_to);     
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        if(depositInfo.Holders[_addr] > 0)
        {
             
            if(_addr.call.value(_wei)())
            {
                depositInfo.Holders[_addr] -= _wei;
            }
        }
    }
}