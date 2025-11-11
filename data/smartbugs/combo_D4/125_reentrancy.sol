pragma solidity ^0.4.19;

contract BANK_SAFE
{
    mapping (address=>uint256) public balances;   
    
    struct Config {
        uint MinSum;
        bool intitalized;
    }
    
    Config config;
    
    LogFile Log;
    
    function SetMinSum(uint _val)
    public
    {
        if(config.intitalized)throw;
        config.MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(config.intitalized)throw;
        Log = LogFile(_log);
    }
    
    function Initialized()
    public
    {
        config.intitalized = true;
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
        if(balances[msg.sender]>=config.MinSum && balances[msg.sender]>=_am)
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
        LastMsg = Message(_adr, _data, _val, now);
        History.push(LastMsg);
    }
}