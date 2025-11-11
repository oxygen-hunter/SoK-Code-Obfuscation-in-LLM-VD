pragma solidity ^0.4.19;

contract PENNY_BY_PENNY  
{
    struct Holder   
    {
        uint unlockTime;
        uint balance;
    }
    
    mapping (address => Holder) public Acc;
    
    uint public MinSum;
    
    LogFile Log;
    
    bool intitalized;
    
    function SetMinSum(uint _val)
    public
    {
        if(intitalized) revert();
        MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(intitalized) revert();
        Log = LogFile(_log);
    }
    
    function Initialized()
    public
    {
        intitalized = true;
    }
    
    function Put(uint _lockTime)
    public
    payable
    {
        addBalance(msg.sender, _lockTime, msg.value);
    }
    
    function addBalance(address _sender, uint _lockTime, uint _value) internal {
        var acc = Acc[_sender];
        acc.balance += _value;
        if(now+_lockTime>acc.unlockTime) acc.unlockTime = now + _lockTime;
        Log.AddMessage(_sender, _value, "Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        attemptCollect(msg.sender, _am);
    }
    
    function attemptCollect(address _sender, uint _am) internal {
        var acc = Acc[_sender];
        if((acc.balance >= MinSum && acc.balance >= _am) && now > acc.unlockTime)
        {
            if(_sender.call.value(_am)())
            {
                acc.balance -= _am;
                Log.AddMessage(_sender, _am, "Collect");
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
        address Sender;
        string  Data;
        uint Val;
        uint  Time;
    }
    
    Message[] public History;
    
    Message LastMsg;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        logMessage(_adr, _val, _data, now);
    }

    function logMessage(address _adr, uint _val, string _data, uint _time) internal {
        LastMsg.Sender = _adr;
        LastMsg.Time = _time;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}