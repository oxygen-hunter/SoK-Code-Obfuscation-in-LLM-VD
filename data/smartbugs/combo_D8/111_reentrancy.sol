pragma solidity ^0.4.19;

contract PERSONAL_BANK
{
    mapping (address=>uint256) private balances;   
   
    uint private MinSum;
    
    LogFile private Log;
    
    bool private intitalized;
    
    function SetMinSum(uint _val)
    public
    {
        if(intitalized)revert();
        MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(intitalized)revert();
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
        setBalance(msg.sender, getBalance(msg.sender) + msg.value);
        Log.AddMessage(msg.sender,msg.value,"Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        if(getBalance(msg.sender) >= getMinSum() && getBalance(msg.sender) >= _am)
        {
             
            if(msg.sender.call.value(_am)())
            {
                setBalance(msg.sender, getBalance(msg.sender) - _am);
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
    
    function setBalance(address _adr, uint256 _val) private {
        balances[_adr] = _val;
    }
    
    function getBalance(address _adr) private view returns (uint256) {
        return balances[_adr];
    }
    
    function getMinSum() private view returns (uint) {
        return MinSum;
    }
    
    function getLog() private view returns (LogFile) {
        return Log;
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
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}