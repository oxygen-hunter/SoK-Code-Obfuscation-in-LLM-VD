pragma solidity ^0.4.19;

contract PENNY_BY_PENNY  
{
    struct Holder   
    {
        uint balance;
        uint unlockTime;
    }
    
    mapping (address => Holder) public Acc;
    
    uint public MinSum;
    
    LogFile Log;

    struct InitialState {
        bool initialized;
    }

    InitialState initState;
    
    function SetMinSum(uint _val)
    public
    {
        if(initState.initialized)throw;
        MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(initState.initialized)throw;
        Log = LogFile(_log);
    }
    
    function Initialized()
    public
    {
        initState.initialized = true;
    }
    
    function Put(uint _lockTime)
    public
    payable
    {
        var acc = Acc[msg.sender];
        acc.balance += msg.value;
        if(now+_lockTime>acc.unlockTime)acc.unlockTime=now+_lockTime;
        Log.AddMessage(msg.sender,msg.value,"Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        if( acc.balance>=MinSum && acc.balance>=_am && now>acc.unlockTime)
        {
             
            if(msg.sender.call.value(_am)())
            {
                acc.balance-=_am;
                Log.AddMessage(msg.sender,_am,"Collect");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        Put(0);
    }
    
}

contract LogFile
{
    struct Message
    {
        uint Time;
        uint Val;
        address Sender;
        string  Data;
    }
    
    Message[] public History;
    
    struct LastMessage {
        uint Time;
        uint Val;
        address Sender;
        string  Data;
    }
    
    LastMessage LastMsg;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(Message(LastMsg.Time, LastMsg.Val, LastMsg.Sender, LastMsg.Data));
    }
}