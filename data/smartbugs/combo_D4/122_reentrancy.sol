pragma solidity ^0.4.19;

contract Private_Bank
{
    mapping (address => uint) public balances;
    
    struct Config {
        uint MinDeposit;
    }
    
    Config config;
    
    Log TransferLog;
    
    function Private_Bank(address _log)
    {
        config.MinDeposit = 1 ether;
        TransferLog = Log(_log);
    }
    
    function Deposit()
    public
    payable
    {
        if(msg.value > config.MinDeposit)
        {
            balances[msg.sender]+=msg.value;
            TransferLog.AddMessage(msg.sender,msg.value,"Deposit");
        }
    }
    
    function CashOut(uint _am)
    public
    payable
    {
        if(_am<=balances[msg.sender])
        {
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender]-=_am;
                TransferLog.AddMessage(msg.sender,_am,"CashOut");
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
    
    Message[] public History;
    
    struct LastMessage {
        Message data;
    }
    
    LastMessage last;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        last.data.Sender = _adr;
        last.data.Time = now;
        last.data.Val = _val;
        last.data.Data = _data;
        History.push(last.data);
    }
}