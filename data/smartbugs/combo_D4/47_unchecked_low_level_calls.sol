pragma solidity ^0.4.19;

contract Ownable
{
    struct Ownership {
        address currentOwner;
        address pendingOwner;
    }
    Ownership ownership = Ownership(msg.sender, address(0));
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        ownership.pendingOwner = addr;
    }
    
    function confirmOwner() 
    public
    {
        if(msg.sender == ownership.pendingOwner)
        {
            ownership.currentOwner = ownership.pendingOwner;
        }
    }
    
    modifier onlyOwner
    {
        if(ownership.currentOwner == msg.sender)_;
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
    uint public MinDeposit;
    mapping (address => uint) public Holders;
    
    function initTokenBank()
    public
    {
        ownership.currentOwner = msg.sender;
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