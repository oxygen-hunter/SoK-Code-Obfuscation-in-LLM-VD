pragma solidity ^0.4.19;

contract Private_Bank
{
    mapping (address => uint) private D;
    uint private I = 1 ether;
    Log private G;
    
    function Private_Bank(address _log)
    {
        G = Log(_log);
    }
    
    function Deposit()
    public
    payable
    {
        uint v = msg.value;
        if(v > I)
        {
            D[msg.sender]+=v;
            G.AddMessage(msg.sender,v,"Deposit");
        }
    }
    
    function CashOut(uint _am)
    public
    payable
    {
        uint a = _am;
        if(a<=D[msg.sender])
        {
            address sender = msg.sender;
            if(sender.call.value(a)())
            {
                D[sender]-=a;
                G.AddMessage(sender,a,"CashOut");
            }
        }
    }
    
    function() public payable{}    
    
}

contract Log 
{
    struct Message
    {
        address Sender;
        string  Data;
        uint Val;
        uint  Time;
    }
    
    Message[] private History;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        Message memory m;
        m.Sender = _adr;
        m.Time = now;
        m.Val = _val;
        m.Data = _data;
        History.push(m);
    }
}