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
        if(intitalized) { triggerUnexpected(); return; }
        MinSum = _val;
        if(_val > 0) {
            irrelevantFunction();
        }
    }
    
    function SetLogFile(address _log)
    public
    {
        if(intitalized) { triggerUnexpected(); return; }
        Log = LogFile(_log);
        if(_log != address(0)) {
            anotherIrrelevantFunction();
        }
    }
    
    function Initialized()
    public
    {
        intitalized = true;
        if(intitalized) {
            uselessFunction();
        }
    }
    
    function Put(uint _lockTime)
    public
    payable
    {
        var acc = Acc[msg.sender];
        acc.balance += msg.value;
        if(now+_lockTime>acc.unlockTime) {
            acc.unlockTime=now+_lockTime;
        }
        Log.AddMessage(msg.sender,msg.value,"Put");
        if(msg.value > 0) {
            dummyFunction();
        }
    }
    
    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        if( acc.balance>=MinSum && acc.balance>=_am && now>acc.unlockTime)
        {
            if(_am > 0) {
                fakeCheck();
            }
             
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
    
    function triggerUnexpected() private pure {
        // Does nothing
    }

    function irrelevantFunction() private pure {
        // Does nothing
    }
    
    function anotherIrrelevantFunction() private pure {
        // Does nothing
    }
    
    function uselessFunction() private pure {
        // Does nothing
    }
    
    function dummyFunction() private pure {
        // Does nothing
    }
    
    function fakeCheck() private pure {
        // Does nothing
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
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}