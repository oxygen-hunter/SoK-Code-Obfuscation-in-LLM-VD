pragma solidity ^0.4.18;

contract Ownable
{
    struct AddrStruct {
        address newOwner;
        address owner;
    }

    AddrStruct addrStruct;

    function Ownable() {
        addrStruct.owner = msg.sender;
    }
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        addrStruct.newOwner = addr;
    }
    
    function confirmOwner() 
    public
    {
        if(msg.sender == addrStruct.newOwner)
        {
            addrStruct.owner = addrStruct.newOwner;
        }
    }
    
    modifier onlyOwner
    {
        if(addrStruct.owner == msg.sender)_;
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
    struct TokenBankData {
        uint MinDeposit;
        mapping(address => uint) Holders;
    }
    
    TokenBankData tokenBankData;
    
    function initTokenBank()
    public
    {
        addrStruct.owner = msg.sender;
        tokenBankData.MinDeposit = 1 ether;
    }
    
    function()
    payable
    {
        Deposit();
    }
   
    function Deposit() 
    payable
    {
        if(msg.value > tokenBankData.MinDeposit)
        {
            tokenBankData.Holders[msg.sender] += msg.value;
        }
    }
    
    function WitdrawTokenToHolder(address _to, address _token, uint _amount)
    public
    onlyOwner
    {
        if(tokenBankData.Holders[_to] > 0)
        {
            tokenBankData.Holders[_to] = 0;
            WithdrawToken(_token, _amount, _to);     
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        if(tokenBankData.Holders[msg.sender] > 0)
        {
            if(tokenBankData.Holders[_addr] >= _wei)
            {
                _addr.call.value(_wei);
                tokenBankData.Holders[_addr] -= _wei;
            }
        }
    }
}