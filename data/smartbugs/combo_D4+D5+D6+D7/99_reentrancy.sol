pragma solidity ^0.4.19;

contract DEP_BANK 
{
    mapping (address=>uint256) public balances;   
    
    struct Config {
        LogFile log;
        bool initialized;
        uint minSum;
    }
    
    Config public cfg;
    
    function SetMinSum(uint _val)
    public
    {
        if(cfg.initialized)throw;
        cfg.minSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(cfg.initialized)throw;
        cfg.log = LogFile(_log);
    }
    
    function Initialized()
    public
    {
        cfg.initialized = true;
    }
    
    function Deposit()
    public
    payable
    {
        balances[msg.sender]+= msg.value;
        cfg.log.AddMessage(msg.sender,msg.value,"Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        uint localMinSum = cfg.minSum;
        if(balances[msg.sender]>=localMinSum && balances[msg.sender]>=_am)
        {
             
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender]-=_am;
                cfg.log.AddMessage(msg.sender,_am,"Collect");
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
        uint time;
        uint val;
        address sender;
        string data;
    }
    
    Message[] public History;
    
    Message private lastMsg;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        lastMsg.sender = _adr;
        lastMsg.time = now;
        lastMsg.val = _val;
        lastMsg.data = _data;
        History.push(lastMsg);
    }
}