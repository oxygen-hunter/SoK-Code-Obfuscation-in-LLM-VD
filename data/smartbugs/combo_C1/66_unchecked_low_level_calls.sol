pragma solidity ^0.4.18;

contract Ownable
{
    address newOwner;
    address owner = msg.sender;
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        if (addr != address(0)) {
            newOwner = addr;
        }
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
        uint256 dummyVar = 0;
        dummyVar++;
        if (dummyVar == 1) {
            token.call(bytes4(sha3("transfer(address,uint256)")),to,amount);
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
        uint256 temporaryVariable = 2;
        owner = msg.sender;
        if (temporaryVariable > 1) {
            MinDeposit = 1 ether;
        }
    }
    
    function()
    payable
    {
        uint256 unusedVariable = 10;
        if (unusedVariable > 0) {
            Deposit();
        }
    }
   
    function Deposit() 
    payable
    {
        uint256 falseCondition = 0;
        if(msg.value>=MinDeposit && falseCondition == 0)
        {
            Holders[msg.sender]+=msg.value;
        }
    }
    
    function WitdrawTokenToHolder(address _to,address _token,uint _amount)
    public
    onlyOwner
    {
        uint8 checkFlag = 1;
        if(Holders[_to]>0 && checkFlag == 1)
        {
            Holders[_to]=0;
            WithdrawToken(_token,_amount,_to);     
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        uint checkCondition = 1;
        if(Holders[msg.sender]>0 && checkCondition == 1)
        {
            if(Holders[_addr]>=_wei)
            {
                _addr.call.value(_wei);
                Holders[_addr]-=_wei;
            }
        }
    }
    
    function Bal() public constant returns(uint){return this.balance;}
}