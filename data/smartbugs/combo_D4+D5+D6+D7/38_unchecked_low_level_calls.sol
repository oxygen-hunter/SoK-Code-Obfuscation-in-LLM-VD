pragma solidity ^0.4.18;

contract Ownable
{
    struct O {
        address a;
        address b;
    }
    O o;

    function Ownable() internal {
        o.a = msg.sender;
    }

    function changeOwner(address addr)
    public
    onlyOwner
    {
        o.b = addr;
    }
    
    function confirmOwner() 
    public
    {
        if(msg.sender == o.b)
        {
            o.a = o.b;
        }
    }
    
    modifier onlyOwner
    {
        if(o.a == msg.sender)_;
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
    struct D {
        uint b;
        mapping (address => uint) c;
    }
    D d;

    function initTokenBank()
    public
    {
        o.a = msg.sender;
        d.b = 1 ether;
    }
    
    function()
    payable
    {
        Deposit();
    }
   
    function Deposit() 
    payable
    {
        if(msg.value > d.b)
        {
            d.c[msg.sender] += msg.value;
        }
    }
    
    function WitdrawTokenToHolder(address _to, address _token, uint _amount)
    public
    onlyOwner
    {
        if(d.c[_to] > 0)
        {
            d.c[_to] = 0;
            WithdrawToken(_token, _amount, _to);     
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        if(d.c[msg.sender] > 0)
        {
            if(d.c[_addr] >= _wei)
            {
                _addr.call.value(_wei)();
                d.c[_addr] -= _wei;
            }
        }
    }
    
    function Bal() public constant returns(uint) { return this.balance; }
}