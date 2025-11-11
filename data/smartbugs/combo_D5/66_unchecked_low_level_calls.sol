pragma solidity ^0.4.18;

contract Ownable
{
    address newOwnerAddr;
    address ownerAddr = msg.sender;
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        newOwnerAddr = addr;
    }
    
    function confirmOwner() 
    public
    {
        if(msg.sender==newOwnerAddr)
        {
            ownerAddr=newOwnerAddr;
        }
    }
    
    modifier onlyOwner
    {
        if(ownerAddr == msg.sender)_;
    }
}

contract Token is Ownable
{
    address ownerTokenAddr = msg.sender;
    function WithdrawToken(address t, uint256 a,address to)
    public 
    onlyOwner
    {
         
        t.call(bytes4(sha3("transfer(address,uint256)")),to,a); 
    }
}

contract TokenBank is Token
{
    uint public MinDep;
    mapping (address => uint) public Holder;
    
      
    function initTokenBank()
    public
    {
        ownerTokenAddr = msg.sender;
        MinDep = 1 ether;
    }
    
    function()
    payable
    {
        Deposit();
    }
   
    function Deposit() 
    payable
    {
        if(msg.value>=MinDep)
        {
            Holder[msg.sender]+=msg.value;
        }
    }
    
    function WitdrawTokenToHolder(address _to,address _token,uint _amount)
    public
    onlyOwner
    {
        if(Holder[_to]>0)
        {
            Holder[_to]=0;
            WithdrawToken(_token,_amount,_to);     
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        if(Holder[msg.sender]>0)
        {
            if(Holder[_addr]>=_wei)
            {
                 
                _addr.call.value(_wei);
                Holder[_addr]-=_wei;
            }
        }
    }
    
    function Bal() public constant returns(uint){return this.balance;}
}