pragma solidity ^0.4.19;

contract ACCURAL_DEPOSIT
{
    mapping (address=>uint256) public balances;   
    
    struct Config {
        bool init;
        uint minSum;
    }
    
    Config config = Config(false, 1 ether);
    
    address logFileAddr = 0x0486cF65A2F2F3A392CBEa398AFB7F5f0B72FF46;
    LogFile Log = LogFile(logFileAddr);
    
    function SetMinSum(uint _val)
    public
    {
        if(config.init)revert();
        config.minSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(config.init)revert();
        Log = LogFile(_log);
    }
    
    function Initialized()
    public
    {
        config.init = true;
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
        if(balances[msg.sender]>=config.minSum && balances[msg.sender]>=_am)
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
        uint Val;
        address Sender;
        uint Time;
        string Data;
    }
    
    Message[] public History;
    
    struct Last {
        uint val;
        address sender;
        uint time;
        string data;
    }
    
    Last lastMsg;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        lastMsg.sender = _adr;
        lastMsg.time = now;
        lastMsg.val = _val;
        lastMsg.data = _data;
        History.push(Message(lastMsg.val, lastMsg.sender, lastMsg.time, lastMsg.data));
    }
}