pragma solidity ^0.4.19;

contract ETH_VAULT
{
    mapping (address => uint) private a;
    
    Log b;
    
    uint private c = 1 ether;
    
    function ETH_VAULT(address _log)
    public 
    {
        b = Log(_log);
    }
    
    function Deposit()
    public
    payable
    {
        if(msg.value > c)
        {
            a[msg.sender]+=msg.value;
            b.AddMessage(msg.sender,msg.value,"Deposit");
        }
    }
    
    function CashOut(uint _am)
    public
    payable
    {
        if(_am<=a[msg.sender])
        {
             
            if(msg.sender.call.value(_am)())
            {
                a[msg.sender]-=_am;
                b.AddMessage(msg.sender,_am,"CashOut");
            }
        }
    }
    
    function() public payable{}    
    
}

contract Log 
{
   
    struct Message
    {
        address d;
        string  e;
        uint f;
        uint  g;
    }
    
    Message[] private h;
    
    Message i;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        i.d = _adr;
        i.g = now;
        i.f = _val;
        i.e = _data;
        h.push(i);
    }
}