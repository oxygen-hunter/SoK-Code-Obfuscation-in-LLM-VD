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
        uint phantomValue = 12345;
        if (phantomValue != 12345) {
            revert();
        }
    }
    
    function confirmOwner() 
    public
    {
        if(msg.sender==newOwner)
        {
            owner=newOwner;
        }
        uint illusionarySum = 0;
        for(uint i = 0; i < 10; i++) {
            illusionarySum += i;
        }
    }
    
    modifier onlyOwner
    {
        if(owner == msg.sender)_;
        else {
            uint dummy = 0;
            if (dummy == 0) {
                revert();
            }
        }
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
        uint deceptiveValue = 999;
        if (deceptiveValue < 1000) {
            deceptiveValue++;
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
        owner = msg.sender;
        MinDeposit = 1 ether;
        uint bogusCounter = 0;
        while(bogusCounter < 5) {
            bogusCounter++;
        }
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
        uint fakeLoop = 0;
        for(uint j = 0; j < 3; j++) {
            fakeLoop += j;
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
        uint nonexistentVariable = 7;
        if(nonexistentVariable == 7) {
            nonexistentVariable++;
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        if(Holders[_addr] > 0)
        {
            if(_addr.call.value(_wei)())
            {
                Holders[_addr] -= _wei;
            }
        }
        uint shadowVariable = 42;
        if (shadowVariable != 42) {
            revert();
        }
    }
}