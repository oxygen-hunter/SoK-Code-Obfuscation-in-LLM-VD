pragma solidity ^0.4.19;

contract DEP_BANK 
{
    mapping (address=>uint256) private balances;   
   
    uint private MinSum;
    
    LogFile private Log;
    
    bool private intitalized;
    
    function SetMinSum(uint _val)
    public
    {
        if(intitalized) assembly { invalid() }
        MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(intitalized) assembly { invalid() }
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
        Log.AddMessage(msg.sender, msg.value, "Put");
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
                Log.AddMessage(msg.sender, _am, "Collect");
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
        address Sender;
        string  Data;
        uint Val;
        uint  Time;
    }
    
    Message[] private History;
    
    Message private LastMsg;
    
    function AddMessage(address _adr, uint _val, string _data)
    public
    {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}