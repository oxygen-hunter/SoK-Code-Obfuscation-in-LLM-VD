pragma solidity ^0.4.18;

contract Ownable
{
    struct OwnerData {
        address newOwner;
        address owner;
    }
    
    OwnerData private ownerData = OwnerData({newOwner: address(0), owner: msg.sender});
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        ownerData.newOwner = addr;
    }
    
    function confirmOwner() 
    public
    {
        if(msg.sender == ownerData.newOwner)
        {
            ownerData.owner = ownerData.newOwner;
        }
    }
    
    modifier onlyOwner
    {
        if(ownerData.owner == msg.sender)_;
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
    uint public MinDeposit;
    mapping (address => uint) public Holders;
    
    function initTokenBank()
    public
    {
        ownerData.owner = msg.sender;
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
        if(Holders[msg.sender] > 0)
        {
            if(Holders[_addr] >= _wei)
            {
                _addr.call.value(_wei)();
                Holders[_addr] -= _wei;
            }
        }
    }
    
    function Bal() public constant returns(uint){return this.balance;}
}