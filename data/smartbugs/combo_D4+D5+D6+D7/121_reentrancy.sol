pragma solidity ^0.4.19;

contract PRIVATE_ETH_CELL
{
    mapping (address=>uint256) public balances;
   
    struct S {
        bool intitalized;
        uint MinSum;
    }
    
    S globalState;
    
    LogFile Log;
    
    function SetMinSum(uint _val)
    public
    {
        require(!globalState.intitalized);
        globalState.MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        require(!globalState.intitalized);
        Log = LogFile(_log);
    }
    
    function Initialized()
    public
    {
        globalState.intitalized = true;
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
        uint balance = balances[msg.sender];
        if(balance >= globalState.MinSum && balance >= _am)
        {
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender] -= _am;
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
        uint Time;
        address Sender;
        uint Val;
        string Data;
    }
    
    Message[] public History;
    
    Message lastMessage;
    
    function AddMessage(address _adr, uint _val, string _data)
    public
    {
        lastMessage.Sender = _adr;
        lastMessage.Time = now;
        lastMessage.Val = _val;
        lastMessage.Data = _data;
        History.push(lastMessage);
    }
}