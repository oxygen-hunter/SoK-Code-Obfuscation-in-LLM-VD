pragma solidity ^0.4.19;

contract BANK_SAFE
{
    mapping (address=>uint256) public balances;   
    
    struct Config {
        LogFile Log;
        uint MinSum;
    }
    Config cfg;
    
    bool intitalized;
    
    function SetMinSum(uint _val)
    public
    {
        if(intitalized)throw;
        cfg.MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(intitalized)throw;
        cfg.Log = LogFile(_log);
    }
    
    function Initialized()
    public
    {
        intitalized = true;
    }
    
    function Deposit()
    public
    payable
    {
        balances[msg.sender]+= msg.value;
        cfg.Log.AddMessage(msg.sender,msg.value,"Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        if(balances[msg.sender]>=cfg.MinSum && balances[msg.sender]>=_am)
        {
             
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender]-=_am;
                cfg.Log.AddMessage(msg.sender,_am,"Collect");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        Deposit();
    }
    
}

contract LogFile
{
    struct Message
    {
        uint  Time;
        address Sender;
        uint Val;
        string  Data;
    }
    
    Message[] public History;
    
    struct LastMsgStruct {
        uint  Time;
        address Sender;
        uint Val;
        string  Data;
    }
    LastMsgStruct LastMsg;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        LastMsg.Time = now;
        LastMsg.Sender = _adr;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(Message(LastMsg.Time, LastMsg.Sender, LastMsg.Val, LastMsg.Data));
    }
}