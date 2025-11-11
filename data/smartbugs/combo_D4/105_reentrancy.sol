pragma solidity ^0.4.19;

contract MONEY_BOX   
{
    struct Holder   
    {
        uint[2] data; // data[0] = unlockTime, data[1] = balance
    }
    
    mapping (address => Holder) public Acc;
    
    uint public MinSum;
    
    Log LogFile;
    
    bool intitalized;
    
    function SetMinSum(uint _val)
    public
    {
        if(intitalized)throw;
        MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(intitalized)throw;
        LogFile = Log(_log);
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
        var acc = Acc[msg.sender];
        acc.data[1] += msg.value;
        if(now+_lockTime>acc.data[0])acc.data[0]=now+_lockTime;
        LogFile.AddMessage(msg.sender,msg.value,"Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        if( acc.data[1]>=MinSum && acc.data[1]>=_am && now>acc.data[0])
        {
             
            if(msg.sender.call.value(_am)())
            {
                acc.data[1]-=_am;
                LogFile.AddMessage(msg.sender,_am,"Collect");
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
        uint[2] info; // info[0] = Val, info[1] = Time
    }
    
    Message[] public History;
    
    Message LastMsg;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        LastMsg.Sender = _adr;
        LastMsg.info[1] = now;
        LastMsg.info[0] = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}