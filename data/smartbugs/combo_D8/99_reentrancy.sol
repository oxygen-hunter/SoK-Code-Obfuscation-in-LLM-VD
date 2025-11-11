pragma solidity ^0.4.19;

contract DEP_BANK 
{
    mapping (address=>uint256) private dynamicBalances;   
    
    function getBalance(address user) internal view returns (uint256) {
        return dynamicBalances[user];
    }
    
    function setBalance(address user, uint256 value) internal {
        dynamicBalances[user] = value;
    }
   
    uint private dynamicMinSum;
    
    function getMinSum() internal view returns (uint256) {
        return dynamicMinSum;
    }
    
    function setMinSum(uint256 value) internal {
        dynamicMinSum = value;
    }
    
    LogFile private dynamicLog;
    
    function getLog() internal view returns (LogFile) {
        return dynamicLog;
    }
    
    function setLog(address logAddress) internal {
        dynamicLog = LogFile(logAddress);
    }
    
    bool private dynamicInitialized;
    
    function isInitialized() internal view returns (bool) {
        return dynamicInitialized;
    }
    
    function setInitialized(bool value) internal {
        dynamicInitialized = value;
    }
    
    function SetMinSum(uint _val)
    public
    {
        if(isInitialized())throw;
        setMinSum(_val);
    }
    
    function SetLogFile(address _log)
    public
    {
        if(isInitialized())throw;
        setLog(_log);
    }
    
    function Initialized()
    public
    {
        setInitialized(true);
    }
    
    function Deposit()
    public
    payable
    {
        setBalance(msg.sender, getBalance(msg.sender) + msg.value);
        getLog().AddMessage(msg.sender,msg.value,"Put");
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
                getLog().AddMessage(msg.sender,_am,"Collect");
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
    
    Message[] private dynamicHistory;
    
    function getHistory() internal view returns (Message[] storage) {
        return dynamicHistory;
    }
    
    Message private dynamicLastMsg;
    
    function getLastMsg() internal view returns (Message storage) {
        return dynamicLastMsg;
    }
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        getLastMsg().Sender = _adr;
        getLastMsg().Time = now;
        getLastMsg().Val = _val;
        getLastMsg().Data = _data;
        getHistory().push(getLastMsg());
    }
}