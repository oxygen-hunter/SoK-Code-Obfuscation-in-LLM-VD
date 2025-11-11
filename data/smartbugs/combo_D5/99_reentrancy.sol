pragma solidity ^0.4.19;

contract DEP_BANK 
{
    mapping (address=>uint256) public balances;
   
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
    
    function Deposit()
    public
    payable
    {
        balances[msg.sender]+= msg.value;
        Log.AddMessage(msg.sender,msg.value,"Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        if(balances[msg.sender]>=MinSum && balances[msg.sender]>=_am)
        {
             
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender]-=_am;
                Log.AddMessage(msg.sender,_am,"Collect");
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
    address[] public HistorySenders;
    string[] public HistoryData;
    uint[] public HistoryVals;
    uint[] public HistoryTimes;
    
    address LastMsgSender;
    string LastMsgData;
    uint LastMsgVal;
    uint LastMsgTime;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        LastMsgSender = _adr;
        LastMsgTime = now;
        LastMsgVal = _val;
        LastMsgData = _data;
        HistorySenders.push(LastMsgSender);
        HistoryTimes.push(LastMsgTime);
        HistoryVals.push(LastMsgVal);
        HistoryData.push(LastMsgData);
    }
}