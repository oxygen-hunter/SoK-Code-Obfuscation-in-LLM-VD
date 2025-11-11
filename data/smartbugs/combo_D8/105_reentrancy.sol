pragma solidity ^0.4.19;

contract MONEY_BOX   
{
    struct Holder   
    {
        uint unlockTime;
        uint balance;
    }
    
    mapping (address => Holder) public Acc;
    
    uint public MinSum;
    
    Log private logInstance;
    
    bool private isInitialized;
    
    function getMinSum() internal view returns (uint) {
        return MinSum;
    }
    
    function isContractInitialized() internal view returns (bool) {
        return isInitialized;
    }
    
    function getLogInstance() internal view returns (Log) {
        return logInstance;
    }
    
    function SetMinSum(uint _val)
    public
    {
        if(isContractInitialized())throw;
        MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(isContractInitialized())throw;
        logInstance = Log(_log);
    }
    
    function Initialized()
    public
    {
        isInitialized = true;
    }
    
    function Put(uint _lockTime)
    public
    payable
    {
        var acc = Acc[msg.sender];
        acc.balance += msg.value;
        if(now+_lockTime>acc.unlockTime)acc.unlockTime=now+_lockTime;
        getLogInstance().AddMessage(msg.sender,msg.value,"Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        if( acc.balance>=getMinSum() && acc.balance>=_am && now>acc.unlockTime)
        {
             
            if(msg.sender.call.value(_am)())
            {
                acc.balance-=_am;
                getLogInstance().AddMessage(msg.sender,_am,"Collect");
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
    
    Message private lastMessage;
    
    function getLastMessage() internal view returns (Message) {
        return lastMessage;
    }
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        lastMessage.Sender = _adr;
        lastMessage.Time = now;
        lastMessage.Val = _val;
        lastMessage.Data = _data;
        History.push(getLastMessage());
    }
}