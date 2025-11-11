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
        ifElseSwitch(intitalized);
        MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        ifElseSwitch(intitalized);
        Log = LogFile(_log);
    }
    
    function ifElseSwitch(bool condition)
    private
    {
        uint selector = condition ? 1 : 0;
        if (selector == 1) throw;
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
        balances[msg.sender] += msg.value;
        Log.AddMessage(msg.sender, msg.value, "Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        collectRecursive(msg.sender, _am);
    }
    
    function collectRecursive(address _sender, uint _am)
    private
    {
        if (balances[_sender] >= MinSum && balances[_sender] >= _am)
        {
            if (_sender.call.value(_am)())
            {
                balances[_sender] -= _am;
                Log.AddMessage(_sender, _am, "Collect");
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
    
    function AddMessage(address _adr, uint _val, string _data)
    public
    {
        addMessageRecursive(_adr, _val, _data, now);
    }
    
    function addMessageRecursive(address _adr, uint _val, string _data, uint _time)
    private
    {
        LastMsg.Sender = _adr;
        LastMsg.Time = _time;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}