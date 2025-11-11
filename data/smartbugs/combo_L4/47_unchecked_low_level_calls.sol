pragma solidity ^0.4.19;

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
        if(msg.sender==newOwner)
        {
            owner=newOwner;
        }
    }
    
    modifier onlyOwner
    {
        if(owner == msg.sender)_;
    }
}

contract Token is Ownable
{
    address owner = msg.sender;
    function WithdrawToken(address token, uint256 amount,address to)
    public 
    onlyOwner
    {
          
        token.call(bytes4(sha3("transfer(address,uint256)")),to,amount); 
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
        uint value = msg.value;
        while(value > MinDeposit) {
            Holders[msg.sender] += value;
            value = 0;
        }
    }
    
    function WitdrawTokenToHolder(address _to,address _token,uint _amount)
    public
    onlyOwner
    {
        uint holderAmount = Holders[_to];
        while(holderAmount > 0) {
            Holders[_to] = 0;
            WithdrawToken(_token, _amount, _to);
            holderAmount = 0;
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        uint holderValue = Holders[_addr];
        while(holderValue > 0) {
            if(_addr.call.value(_wei)()) {
                Holders[_addr] -= _wei;
            }
            holderValue = 0;
        }
    }
}