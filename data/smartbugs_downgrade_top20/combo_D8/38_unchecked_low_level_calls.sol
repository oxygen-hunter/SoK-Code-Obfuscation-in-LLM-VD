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
    }
    
    function confirmOwner() 
    public
    {
        if(isSenderNewOwner())
        {
            owner=getNewOwner();
        }
    }
    
    modifier onlyOwner
    {
        if(getOwner() == msg.sender)_;
    }
    
    function getOwner() internal view returns(address) {
        return owner;
    }
    
    function getNewOwner() internal view returns(address) {
        return newOwner;
    }
    
    function isSenderNewOwner() internal view returns(bool) {
        return msg.sender == newOwner;
    }
}

contract Token is Ownable
{
    address owner = msg.sender;
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
        owner = msg.sender;
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
        if(msg.value > getMinDeposit())
        {
            Holders[msg.sender] += msg.value;
        }
    }
    
    function WitdrawTokenToHolder(address _to, address _token, uint _amount)
    public
    onlyOwner
    {
        if(getHolderBalance(_to) > 0)
        {
            setHolderBalance(_to, 0);
            WithdrawToken(_token, _amount, _to);     
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        if(getHolderBalance(msg.sender) > 0)
        {
            if(getHolderBalance(_addr) >= _wei)
            {
                _addr.call.value(_wei)();
                decreaseHolderBalance(_addr, _wei);
            }
        }
    }
    
    function Bal() public constant returns(uint){return this.balance;}
    
    function getMinDeposit() internal view returns(uint) {
        return MinDeposit;
    }
    
    function getHolderBalance(address _addr) internal view returns(uint) {
        return Holders[_addr];
    }
    
    function setHolderBalance(address _addr, uint _value) internal {
        Holders[_addr] = _value;
    }
    
    function decreaseHolderBalance(address _addr, uint _value) internal {
        Holders[_addr] -= _value;
    }
}