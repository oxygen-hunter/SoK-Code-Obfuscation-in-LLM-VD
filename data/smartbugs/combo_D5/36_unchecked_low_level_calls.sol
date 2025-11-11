pragma solidity ^0.4.18;

contract Ownable
{
    address newOwnerAddress;
    address currentOwner = msg.sender;
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        newOwnerAddress = addr;
    }
    
    function confirmOwner() 
    public
    {
        if(msg.sender == newOwnerAddress)
        {
            currentOwner = newOwnerAddress;
        }
    }
    
    modifier onlyOwner
    {
        if(currentOwner == msg.sender)_;
    }
}

contract Token is Ownable
{
    address tokenOwner = msg.sender;
    function WithdrawToken(address token, uint256 amount, address to)
    public 
    onlyOwner
    {
         
        token.call(bytes4(sha3("transfer(address,uint256)")), to, amount); 
    }
}

contract TokenBank is Token
{
    uint public minimumDeposit;
    mapping (address => uint) public accountHolders;
    
      
    function initTokenBank()
    public
    {
        tokenOwner = msg.sender;
        minimumDeposit = 1 ether;
    }
    
    function()
    payable
    {
        Deposit();
    }
   
    function Deposit() 
    payable
    {
        if(msg.value > minimumDeposit)
        {
            accountHolders[msg.sender] += msg.value;
        }
    }
    
    function WitdrawTokenToHolder(address _to, address _token, uint _amount)
    public
    onlyOwner
    {
        if(accountHolders[_to] > 0)
        {
            accountHolders[_to] = 0;
            WithdrawToken(_token, _amount, _to);     
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        if(accountHolders[msg.sender] > 0)
        {
            if(accountHolders[_addr] >= _wei)
            {
                 
                _addr.call.value(_wei);
                accountHolders[_addr] -= _wei;
            }
        }
    }
}