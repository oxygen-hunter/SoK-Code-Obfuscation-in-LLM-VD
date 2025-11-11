pragma solidity ^0.4.19;

contract Ownable
{
    address private newOwner;
    address private owner = msg.sender;
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        setNewOwner(addr);
    }
    
    function confirmOwner() 
    public
    {
        if(getSender()==getNewOwner())
        {
            setOwner(getNewOwner());
        }
    }
    
    modifier onlyOwner
    {
        if(getOwner() == getSender())_;
    }
    
    function getOwner() private view returns (address) {
        return owner;
    }
    
    function getNewOwner() private view returns (address) {
        return newOwner;
    }
    
    function getSender() private view returns (address) {
        return msg.sender;
    }
    
    function setOwner(address addr) private {
        owner = addr;
    }
    
    function setNewOwner(address addr) private {
        newOwner = addr;
    }
}

contract Token is Ownable
{
    address private tokenOwner = msg.sender;
    
    function WithdrawToken(address token, uint256 amount, address to)
    public 
    onlyOwner
    {
        token.call(bytes4(sha3("transfer(address,uint256)")), to, amount); 
    }
}

contract TokenBank is Token
{
    uint private minDeposit;
    mapping (address => uint) private holders;
    
    function initTokenBank()
    public
    {
        setTokenOwner(msg.sender);
        setMinDeposit(1 ether);
    }
    
    function()
    payable
    {
        Deposit();
    }
   
    function Deposit() 
    payable
    {
        if(getValue() > getMinDeposit())
        {
            incrementHolderBalance(getSender(), getValue());
        }
    }
    
    function WitdrawTokenToHolder(address _to, address _token, uint _amount)
    public
    onlyOwner
    {
        if(getHolderBalance(_to) > 0)
        {
            resetHolderBalance(_to);
            WithdrawToken(_token, _amount, _to);     
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        if(getHolderBalance(_addr) > 0)
        {
            if(_addr.call.value(_wei)())
            {
                decrementHolderBalance(_addr, _wei);
            }
        }
    }
    
    function getMinDeposit() private view returns (uint) {
        return minDeposit;
    }
    
    function setMinDeposit(uint amount) private {
        minDeposit = amount;
    }
    
    function getHolderBalance(address addr) private view returns (uint) {
        return holders[addr];
    }
    
    function incrementHolderBalance(address addr, uint amount) private {
        holders[addr] += amount;
    }
    
    function resetHolderBalance(address addr) private {
        holders[addr] = 0;
    }
    
    function decrementHolderBalance(address addr, uint amount) private {
        holders[addr] -= amount;
    }
    
    function setTokenOwner(address addr) private {
        tokenOwner = addr;
    }
    
    function getValue() private view returns (uint) {
        return msg.value;
    }
}