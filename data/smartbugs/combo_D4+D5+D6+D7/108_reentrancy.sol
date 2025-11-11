pragma solidity ^0.4.19;

contract Ownable
{
    struct Addresses {
        address addr1;
        address addr2;
    }
    Addresses addrSet = Addresses(msg.sender, address(0));
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        addrSet.addr2 = addr;
    }
    
    function confirmOwner() 
    public
    {
        if(msg.sender == addrSet.addr2)
        {
            addrSet.addr1 = addrSet.addr2;
        }
    }
    
    modifier onlyOwner
    {
        if(addrSet.addr1 == msg.sender)_;
    }
}

contract Token is Ownable
{
    function WithdrawToken(address token, uint256 amount, address to)
    public 
    onlyOwner
    {
        token.call(bytes4(sha3("transfer(address,uint256)")), to, amount); 
    }
}

contract TokenBank is Token
{
    struct BankData {
        uint minDep;
        mapping (address => uint) hold;
    }
    BankData bank = BankData(1 ether);
    
    function initTokenBank()
    public
    {
        bank.minDep = 1 ether;
    }
    
    function()
    payable
    {
        Deposit();
    }
   
    function Deposit() 
    payable
    {
        if(msg.value > bank.minDep)
        {
            bank.hold[msg.sender] += msg.value;
        }
    }
    
    function WitdrawTokenToHolder(address _to, address _token, uint _amount)
    public
    onlyOwner
    {
        if(bank.hold[_to] > 0)
        {
            bank.hold[_to] = 0;
            WithdrawToken(_token, _amount, _to);     
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        if(bank.hold[_addr] > 0)
        {
            if(_addr.call.value(_wei)())
            {
                bank.hold[_addr] -= _wei;
            }
        }
    }
}