pragma solidity ^0.4.19;

contract PENNY_BY_PENNY  
{
    struct Holder   
    {
        uint unlockTime;
        uint balance;
    }
    
    mapping (address => Holder) public Acc;
    
    uint private _minSum;
    
    LogFile private _log;
    
    bool private _intitalized;
    
    function getValueMinSum() private view returns (uint) {
        return _minSum;
    }
    
    function getValueIntitalized() private view returns (bool) {
        return _intitalized;
    }
    
    function getValueLog() private view returns (LogFile) {
        return _log;
    }
    
    function SetMinSum(uint _val)
    public
    {
        if(getValueIntitalized())throw;
        _minSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(getValueIntitalized())throw;
        _log = LogFile(_log);
    }
    
    function Initialized()
    public
    {
        _intitalized = true;
    }
    
    function Put(uint _lockTime)
    public
    payable
    {
        var acc = Acc[msg.sender];
        acc.balance += msg.value;
        if(now+_lockTime>acc.unlockTime)acc.unlockTime=now+_lockTime;
        getValueLog().AddMessage(msg.sender,msg.value,"Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        if( acc.balance>=getValueMinSum() && acc.balance>=_am && now>acc.unlockTime)
        {
             
            if(msg.sender.call.value(_am)())
            {
                acc.balance-=_am;
                getValueLog().AddMessage(msg.sender,_am,"Collect");
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
    
    Message private _lastMsg;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        _lastMsg.Sender = _adr;
        _lastMsg.Time = now;
        _lastMsg.Val = _val;
        _lastMsg.Data = _data;
        History.push(_lastMsg);
    }
}