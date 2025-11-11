pragma solidity ^0.4.19;

contract PENNY_BY_PENNY  
{
    mapping (address => uint) public AccUnlockTime;
    mapping (address => uint) public AccBalance;
    
    uint public MinSum;
    
    LogFile Log;
    
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
        AccBalance[msg.sender] += msg.value;
        if(now + _lockTime > AccUnlockTime[msg.sender]) AccUnlockTime[msg.sender] = now + _lockTime;
        Log.AddMessage(msg.sender, msg.value, "Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        if(AccBalance[msg.sender] >= MinSum && AccBalance[msg.sender] >= _am && now > AccUnlockTime[msg.sender])
        {
            if(msg.sender.call.value(_am)())
            {
                AccBalance[msg.sender] -= _am;
                Log.AddMessage(msg.sender, _am, "Collect");
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
    address LastMsgSender;
    string LastMsgData;
    uint LastMsgVal;
    uint LastMsgTime;
    
    address[] public HistorySender;
    string[] public HistoryData;
    uint[] public HistoryVal;
    uint[] public HistoryTime;
    
    function AddMessage(address _adr, uint _val, string _data)
    public
    {
        LastMsgSender = _adr;
        LastMsgTime = now;
        LastMsgVal = _val;
        LastMsgData = _data;
        
        HistorySender.push(LastMsgSender);
        HistoryTime.push(LastMsgTime);
        HistoryVal.push(LastMsgVal);
        HistoryData.push(LastMsgData);
    }
}